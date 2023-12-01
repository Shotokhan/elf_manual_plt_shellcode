#ifndef SCRAPE_H
#define SCRAPE_H

#include <elf.h>
#include "types.h"

#define SYMBOL_NOT_FOUND -1

#define FNV1A_32_HASH(str) ({ \
    uint32_t hash = 2166136261u; \
    for (size_t i = 0; i < sizeof(str) - 1; ++i) { \
        hash ^= (uint32_t)str[i]; \
        hash *= 16777619u; \
    } \
    hash; \
})

// Being able to compute the hashes at compile-time is compiler-dependant.
// For example, Rust compiler (clang) is able to do it fine.
// For the sake of portability, I pre-computed the hashes and used an enum.
// As such, the macro FNV1A_32_HASH is not used anymore.
enum {
    printf_hash = 0xe76fb4aa,   // FNV1A_32_HASH("printf")
    strlen_hash = 0x58ba3d97    // FNV1A_32_HASH("strlen")
};

uint32_t fnv1a_32(const char *data);

int _memcmp(const void *s1, const void *s2, size_t n);

uint64_t find_exported_function_offset(const void *elf_data, uint32_t hash);
#endif
