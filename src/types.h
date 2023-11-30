#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef uint32_t size_t;
typedef int off_t;

typedef struct stat {
    unsigned int    st_dev;
    unsigned int    st_ino;
    unsigned int    st_mode;
    unsigned int    st_nlink;
    unsigned int    st_uid;
    unsigned int    st_gid;
    unsigned int    st_rdev;
    long            st_size;
    unsigned long   st_atime;
    unsigned long   st_mtime;
    unsigned long   st_ctime;
    unsigned int    st_blksize;
    unsigned int    st_blocks;
    unsigned int    st_flags;
    unsigned int    st_gen;
} stat;
#endif
