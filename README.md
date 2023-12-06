# elf_manual_plt_shellcode
Example of run-time manual PLT in C.

**TL;DR**: *Write a C program that calls libc functions, compile it to a shellcode, load it in memory. Featuring function scraping from ELF as "procedure linkage", code & compilation tricks, and more.*

## Usage
To repeat the build and the test, you can use the provided `Makefile`:

- `make all` or just `make` to build `src`: the `bin` directory will be created, and the shellcode will be extracted to `bin/shellcode`; also, versions of the compiler and the linker (libc) are dumped to `bin/versions.txt`.
- `make inspect` to disassemble both the executable `bin/main` and the shellcode `bin/shellcode`: it's useful to compare them and check that the shellcode was extracted successfully; also, it's important to verify that the code does not rely on other sections other than the `.text` area itself.
- `make loader` to build `test`: it will be created the executable `bin/loader`.
- Run `bin/loader bin/shellcode` to load the shellcode and run it.
- `make clean` to clean-up the build directories.

If everything works well, you can then modify the code and try other things on your own, or emulate the style for other use cases. The techniques used are described below.

## Version
Tested with the following version of `gcc` and `libc`:

```
gcc (GCC) 12.3.1 20230508 (Red Hat 12.3.1-1)
ldd (GNU libc) 2.36
```

When running `make all` (or just `make`), the versions are dumped to `bin/versions.txt`.

If you encounter issues with other versions and want to make them work, open an *Issue* for the specific version: public discussion can help everyone. You can also just open an *Issue* for a different version that works well, putting in the `.text` area as output of `objdump -D bin/main`. The same for other compilers and/or architectures.

> After doing any changes, make sure that `_start` is the first function in the `.text` area of the compiled binary, that the code does not make use of global area and that it's statically linked.

## Techniques
In this paragraph, code techniques and compilation options necessary to compile the application as a shellcode and make it use libc functions are described. They're not just necessary, but also designed to make the development process easy (compared to writing the shellcode in assembly).

### Re-defining headers
In order to use libc functions, the shellcode needs to call them properly. So, first of all, the compiler needs the function signatures, to be used for the function pointers. Then, function parameters and return values have specific types, that must be defined. *Values* of parameters and return values are specified as enums.

#### Enums
It was defined a header file `src/enums.h`. For example, for the page protection attributes to be passed to `mmap` function:

```c
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
```

#### Types
It was defined a header file `src/types.h`. Some common types that are used in many libc functions:

```c
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef uint32_t size_t;
typedef unsigned int off_t;
```

#### Functions
It was defined a header file `src/functions.h`. Function pointers are defined with `typedef`, and the calling convention is specified with `__attribute__((__cdecl__))`, despite on some compiler versions it's ignored.

```c
typedef int (__attribute__((__cdecl__)) *printf)(const char* __format, ...);
```

Note that the token `...` is used to specify the variable number of arguments in the `printf`.

This definition is then used in a header file `src/module.h`, containing a struct for all functions:

```c
typedef struct {
    printf _printf;
} FuncStruct;
```

This struct is then used in `src/main.c`, and the addresses of function pointers are filled at runtime, calculated from the libc base address.

### Using stack for constants (no global variables)
In order to extract the shellcode from the binary, the easiest way is to have all in the `.text` area. As such, there must not be any global variable. For most types, it's enough to use local variables. Strings are placed in global area even if used as local variables, when declared as `char* string = "this_string";`. To give a hint to the compiler, such that the string is placed on the stack, it must be declared like this:

```c
    // NOTE: using "char filename[]" results in the string on the stack;
    // whereas using "char* filename" results in the string in ".rodata"
    char filename[] = "/lib64/libc.so.6";
```

### Direct syscall invocation using a wrapper
Since the shellcode is a user-mode shellcode, it is able to invoke system calls. It's useful because the shellcode needs to be able to *exit* gracefully and because it needs to read and `mmap` libc binary from disk, to find the function offsets to be applied to the libc base address.

In `/src/load_libc.h`, the `syscall stub` is defined as an `extern` function with a variable number of arguments:

```c
extern long _syscall(long syscall_number, ...);
```

It is then implemented in an assembly file, `src/syscall.S`. It basically performs argument shifting, and then calls the `syscall` instruction:

```c
    .text
    .globl _syscall

_syscall:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	movq %r9, %r8
	movq 8(%rsp),%r9	/* arg6 is on the stack.  */
	syscall			    /* Do the system call.  */
	ret			        /* Return to caller.  */
```

Syscall numbers are defined in `src/enums.h`, and syscall are called as:

```c
    int fd = _syscall(SYS_open, filename, O_RDONLY);
```

### Scraping of function addresses in ELF header
To call libc functions using function pointers, the shellcode needs to compute their addresses from libc base address. So, it needs the offset of each function in libc binary. The technique used for that, in this case, is to parse the ELF header, access section headers and in particular search the dynamic symbol in `.dynsym` section. This process is referred to as "scraping".

In order to perform the scraping, it's mandatory to load libc from disk, since the section headers are stripped from the libc executable image at runtime.

Symbol names are strings but, in order to reduce the shellcode size and obfuscate it, the symbol name comparison is performed using hashing: the hash of the target symbol name is compared with the hash of the current symbol name, at each iteration, until the symbol is found. The hash function used is `fnv1a_32`, which works well for strings and produces a value that is stored as a 32-bit integer. Reference about the hash function: [FNV hash](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function).

#### Compile-time string hashing
In source code, the *FNV* hash function was implemented both as a function and as a macro. The macro is in `src/scrape.h`:

```c
#define FNV1A_32_HASH(str) ({ \
    uint32_t hash = 2166136261u; \
    for (size_t i = 0; i < sizeof(str) - 1; ++i) { \
        hash ^= (uint32_t)str[i]; \
        hash *= 16777619u; \
    } \
    hash; \
})
```

The idea was to define an enum like this:

```c
enum {
    printf_hash = FNV1A_32_HASH("printf"),
};
```

Anyway, having the macro computed at compile-time by the preprocessor is compiler-dependent: for the version used, it was compiled as an inline function. So, at last, the hash was computed offline and placed in the code, like:

```c
enum {
    printf_hash = 0xe76fb4aa,   // FNV1A_32_HASH("printf")
};
```

Open a *Pull Request* if you are able to make the `FNV1A_32_HASH` directive work at compile-time for this version, or open a *Issue* if it works for another version and/or compiler just as it is.

### Defining cookies for later shellcode patching
The shellcode needs to call libc functions, and is able to compute function offsets for a given libc binary. So, it needs the base address for a libc executable that is already loaded in memory and initialized. This address will be put in the shellcode via patching, i.e., modifying some bytes of the shellcode before running it. To make patching easier, instead of hard-coding the offset at which the paching must be done, it's possible to define a byte sequence that can be searched & replaced at runtime by the software component that loads the shellcode. In this case, there was only the need for one byte sequence, in `src/main.c`:

```c
    // runtime_libc_base contains a "cookie" to be patched with the actual libc base address when injecting the code
    void* runtime_libc_base = (void*)0xdeadbeefcafebabe;
```

### Compilation and linking options
In addition to code style and techniques, it is important to control how the actual code is generated and how it is organized in the binary.

#### Exclude libc
By default, C compilation always includes libc, both for statically linked and dynamically linked binaries. But we want to generate a shellcode, so we need a de-bloated binary. It's possible to exclude libc by adding this option in `CFLAGS`, in the `Makefile`:

```-nostdlib```

#### Strip unused functions
Since shellcodes usually have strict space requirements, it's important to reduce size by removing unused code, such as unused functions. A way to do that is to assign to each function its own section using the option `-ffunction-sections` and then perform garbage collection of sections using the option `-Wl,--gc-sections`. So, the following options were added to `CFLAGS`, in the `Makefile`:

```-ffunction-sections -Wl,--gc-sections```

#### Overwrite _start function
The C runtime defines its own `_start` function. For a shellcode, it's enough to have a simpler one, that just calls the `main()` function and calls the *exit* system call with the return code from the main function.

To do that, it was implemented a `_start` function in `src/main.c`, and the `-nostartfiles` option was added to `LDFLAGS`, in the `Makefile`.

#### Custom linker script to have _start as first function
This is important for exporting the shellcode from the binary, since the shellcode is literally the `.text` area. So, the `.text` area must start with the `_start` function.

It was implemented by adding the `-T linker_script.ld` option to `LDFLAGS`, in the `Makefile`, and by implementing the linker script `linker_script.ld`; also, the signature of `_start` has an attribute for an explicit section name:

```void __attribute__((section(".text.startup"))) _start()```

This section name is then used in the linker script:

```c
ENTRY(_start);

SECTIONS {
    . = 0x0400200;

    .text : {
        *(.text.startup)
        *(.text.*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    .rodata : {
        *(.rodata)
    }

    .symtab : {
        *(.symtab)
    }

    .strtab : {
        *(.strtab)
    }

    .shstrtab : {
        *(.shstrtab)
    }
}
```

Note that empty sections are then stripped out thanks to the `-Wl,--gc-sections` appended to `CFLAGS`.

### Shellcode extraction & loading
#### Extraction
The shellcode is extracted with the tool `objcopy`: the bytes of the `.text` area are dumped to file, more specifically to `bin/shellcode`. In particular:

```sh
$ objcopy --dump-section .text=bin/shellcode bin/main
```

Since `bin/shellcode` is not an ELF, the tool `objdump` is not able to disassemble it automatically: to disassemble it, it's mandatory to specify additional information about the architecture. In particular:

```sh
$ objdump -M intel,x86-64 -b binary -D -mi386 bin/shellcode
```

#### Loading
The program `test/loader.c` performs the following:

- Accept as input from `argv` the filename of the shellcode to read.
- Read the shellcode into a buffer allocated with `mmap` having *RWX* permissions.
- Read the libc base address from `/proc/self/maps`.
- Patch the shellcode cookie with libc base address.
- Jump to the shellcode.

## Related
- Another approach for libc could be to use the functions `dlopen`, `dlsym` and similar, that means properly loading a shared object (like libc) at runtime. However, these functions are implemented in libc, so, if the shellcode was able to call them, it would be already able to call libc functions. Maybe the approach could be to start with a few function leaks and load a new instance of libc with the ability of having all symbol information, without scraping the ELF header. It's completely an open question.
- For the Linux kernel, the section headers are different: they have custom names, for example there isn't any `.dynsym` header. However, it's possible to use the function `kallsyms_lookup_name` to get the address of a symbol, given the symbol name. So, a kernel shellcode for Linux, capable of getting addresses of functions and call them, would start from the leak of `kallsyms_lookup_name`, and then fill its own structure of function pointers to implement its logic.
- For the Windows kernel, a similar project, which build the shellcode from *Rust*, is [SassyKitdi](https://github.com/wumb0/zerosum0x0_SassyKitdi). The related blog post is [here](https://zerosum0x0.blogspot.com/2020/08/sassykitdi-kernel-mode-tcp-sockets.html).
