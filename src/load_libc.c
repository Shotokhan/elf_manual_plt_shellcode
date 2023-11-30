#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "load_libc.h"


void* load_libc() {
    // NOTE: using "char filename[]" results in the string on the stack;
    // whereas using "char* filename" results in the string in ".rodata"
    char filename[] = "/lib64/libc.so.6";

    // NOTE: syscall is compiled "inline", by using the syscall instruction;
    // in fact, it's not compiled as a function
    // NOTE: constants are compiled "inline" as well
    int fd = syscall(SYS_open, filename, O_RDONLY);
    if (fd == -1) {
        syscall(SYS_close, fd);
        return NULL;
    }

    struct stat file_stat;
    if (syscall(SYS_fstat, fd, &file_stat) == -1) {
        syscall(SYS_close, fd);
        return NULL;
    }
    off_t file_size = file_stat.st_size;

    void* libc_base = syscall(SYS_mmap, NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (libc_base == MAP_FAILED) {
        syscall(SYS_close, fd);
        return NULL;
    }
    syscall(SYS_close, fd);

    return libc_base;
}
