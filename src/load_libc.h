#ifndef LOAD_LIBC_H
#define LOAD_LIBC_H

void* load_libc();

extern long _syscall(long syscall_number, ...);
#endif
