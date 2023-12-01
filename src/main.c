#include "functions.h"
#include "load_libc.h"
#include "scrape.h"
#include "module.h"
#include "enums.h"
#include "types.h"
#include "main.h"


__attribute__((force_align_arg_pointer))
void _start() {
    int result = main();

    _syscall(SYS_exit, result);
    __builtin_unreachable();  // tell the compiler to make sure side effects are done before the asm statement
}

int main() {
    void* libc_base = load_libc();
    if (libc_base == NULL) {
        return 1;
    }
    // TODO: the exported function is not found (properly), fix it
    // TODO: tweak compilation options in Makefile (also to optimize code size, like removing unused code)
    // TODO: analyze the binary to check that the code in .text area does not need other sections
    // TODO: develop a script to extract the shellcode from the binary
    // TODO: develop a loader for the shellcode
    // TODO: describe ALL the techniques used in terms of code and compilation to achieve the result
    uint64_t printf_offset = find_exported_function_offset(libc_base, printf_hash);
    if (printf_offset == SYMBOL_NOT_FOUND) {
        return 2;
    }
    uint64_t strlen_offset = find_exported_function_offset(libc_base, strlen_hash);
    if (strlen_offset == SYMBOL_NOT_FOUND) {
        return 3;
    }
    FuncStruct func_struct;
    func_struct._printf = libc_base + printf_offset;
    func_struct._strlen = libc_base + strlen_offset;
    char program_name[] = "runtime_plt";
    char _format[] = "Program name: %s, length: %d\n";
    func_struct._printf(_format, program_name, func_struct._strlen(program_name));
    return 0;
}
