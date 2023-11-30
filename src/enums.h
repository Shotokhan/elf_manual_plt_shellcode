#ifndef ENUMS_H
#define ENUMS_H

#define NULL (void*) 0

#define SYS_open 2
#define SYS_close 3
#define SYS_fstat 5
#define SYS_mmap 9
#define SYS_exit 60

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_PRIVATE 2
#define MAP_FAILED (void*) -1

#define O_RDONLY 0
#endif
