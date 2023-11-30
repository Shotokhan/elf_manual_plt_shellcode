#include "enums.h"
#include "types.h"
#include "load_libc.h"


void* load_libc() {
    // NOTE: using "char filename[]" results in the string on the stack;
    // whereas using "char* filename" results in the string in ".rodata"
    char filename[] = "/lib64/libc.so.6";

    // NOTE: constants are compiled "inline"
    int fd = _syscall(SYS_open, filename, O_RDONLY);
    if (fd == -1) {
        _syscall(SYS_close, fd);
        return NULL;
    }

    stat file_stat;
    if (_syscall(SYS_fstat, fd, &file_stat) == -1) {
        _syscall(SYS_close, fd);
        return NULL;
    }
    off_t file_size = file_stat.st_size;

    void* libc_base = _syscall(SYS_mmap, NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (libc_base == MAP_FAILED) {
        _syscall(SYS_close, fd);
        return NULL;
    }
    _syscall(SYS_close, fd);

    return libc_base;
}
