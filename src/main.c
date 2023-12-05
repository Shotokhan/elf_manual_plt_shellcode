#include "main.h"
#include "functions.h"
#include "load_libc.h"
#include "scrape.h"
#include "module.h"
#include "enums.h"
#include "types.h"


__attribute__((force_align_arg_pointer))
void __attribute__((section(".text.startup"))) _start() {
    int result = main();

    _syscall(SYS_exit, result);
    __builtin_unreachable();  // tell the compiler to make sure side effects are done before the asm statement
}

int main() {
    // TODO: describe ALL the techniques used in terms of code and compilation to achieve the results

    // mmaped_libc_base is only for the function scraping. In fact, in order to call libc functions, libc needs to be initialized.
    // Calling libc functions of a library that was only mmap-ed results in segmentation faults.
    // At the same time, the libc that is loaded at runtime has got the section headers stripped, so the scraping can't be done on it.
    void* mmaped_libc_base = load_libc();
    if (mmaped_libc_base == NULL) {
        return 1;
    }
    // runtime_libc_base contains a "cookie" to be patched with the actual libc base address when injecting the code
    void* runtime_libc_base = (void*)0xdeadbeefcafebabe;
    uint64_t printf_offset = find_exported_function_offset(mmaped_libc_base, printf_hash);
    if (printf_offset == SYMBOL_NOT_FOUND) {
        return 2;
    }
    FuncStruct func_struct;
    func_struct._printf = runtime_libc_base + printf_offset;
    char program_name[] = "runtime_plt";
    char _format[] = "Program name: %s, length: %d\n";
    func_struct._printf(_format, program_name, sizeof(program_name));
    return 0;
}
