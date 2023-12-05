#include "scrape.h"
#include "enums.h"


uint32_t fnv1a_32(const char *data) {
    const uint32_t FNV_prime = 16777619;
    const uint32_t FNV_offset_basis = 2166136261;

    uint32_t hash = FNV_offset_basis;

    while (*data != '\0') {
        hash ^= (uint32_t)(*data++);
        hash *= FNV_prime;
    }

    return hash;
}

/*
int _memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] < p2[i]) return -1;
        else if (p1[i] > p2[i]) return 1;
    }
    return 0;
}
*/

uint64_t find_exported_function_offset(const void *elf_data, uint32_t hash) {
    const Elf64_Ehdr *elf_header = (const Elf64_Ehdr *)elf_data;

    const Elf64_Shdr *section_headers = (const Elf64_Shdr *)(elf_data + elf_header->e_shoff);

    // Find the section containing the dynamic symbol table (usually .dynsym)
    const Elf64_Shdr *dynsym_header = NULL;
    for (int i = 0; i < elf_header->e_shnum; ++i) {
        if (section_headers[i].sh_type == SHT_DYNSYM) {
            dynsym_header = &section_headers[i];
            break;
        }
    }

    if (!dynsym_header) {
        return SYMBOL_NOT_FOUND; // No dynamic symbol table found
    }

    const Elf64_Sym *dynsym = (const Elf64_Sym *)(elf_data + dynsym_header->sh_offset);

    // Find the symbol by hash
    for (int i = 0; i < dynsym_header->sh_size / sizeof(Elf64_Sym); ++i) {
        const char *symbol_name = (const char *)(elf_data + section_headers[dynsym_header->sh_link].sh_offset + dynsym[i].st_name);
        if (fnv1a_32(symbol_name) == hash && ELF64_ST_TYPE(dynsym[i].st_info) == STT_FUNC) {
            return (uint64_t)dynsym[i].st_value;
        }
    }

    return SYMBOL_NOT_FOUND; // Symbol not found
}
