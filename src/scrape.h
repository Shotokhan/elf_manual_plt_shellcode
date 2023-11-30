#include <elf.h>
#include <stddef.h>

#define SYMBOL_NOT_FOUND -1

#define FNV1A_32_HASH(str) ({ \
    uint32_t hash = 2166136261u; \
    for (size_t i = 0; i < sizeof(str) - 1; ++i) { \
        hash ^= (uint32_t)str[i]; \
        hash *= 16777619u; \
    } \
    hash; \
})

uint32_t fnv1a_32(const char *data);

int memcmp(const void *s1, const void *s2, size_t n);

uint64_t find_exported_function_offset(const void *elf_data, uint32_t hash);
