#include "functions.h"
#include "load_libc.h"
#include "scrape.h"
#include "module.h"


int main(int argc, char* argv[]) {
    void* libc_base = load_libc();
    uint64_t printf_offset = find_exported_function_offset(libc_base, FNV1A_32_HASH("printf"));
    uint64_t strlen_offset = find_exported_function_offset(libc_base, FNV1A_32_HASH("strlen"));
    FuncStruct func_struct;
    func_struct._printf = libc_base + printf_offset;
    func_struct._strlen = libc_base + strlen_offset;
    char _format[] = "Program name: %s, length: %d\n";
    func_struct._printf(_format, argv[0], func_struct._strlen(argv[0]));
}
