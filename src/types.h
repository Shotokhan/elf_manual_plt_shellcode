#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef uint32_t size_t;
typedef unsigned int off_t;

// From /usr/include/asm/stat.h in Linux
// Anyway, it changes across architectures and I'm having some troubles, so I don't use it
typedef struct stat64 {
    unsigned long long      st_dev;
    unsigned char   __pad0[4];
    unsigned long   __st_ino;
    unsigned int    st_mode;
    unsigned int    st_nlink;
    unsigned long   st_uid;
    unsigned long   st_gid;
    unsigned long long      st_rdev;
    unsigned char   __pad3[4];
    long long       st_size;
    unsigned long   st_blksize;
    /* Number 512-byte blocks allocated. */
    unsigned long long      st_blocks;
    unsigned long   st_atime;
    unsigned long   st_atime_nsec;
    unsigned long   st_mtime;
    unsigned int    st_mtime_nsec;
    unsigned long   st_ctime;
    unsigned long   st_ctime_nsec;
    unsigned long long      st_ino;
} stat;
#endif
