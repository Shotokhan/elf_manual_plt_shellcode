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

int _memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] < p2[i]) return -1;
        else if (p1[i] > p2[i]) return 1;
    }
    return 0;
}

uint64_t find_exported_function_offset(const void *elf_data, uint32_t hash) {
    const Elf64_Ehdr *elf_header = (const Elf64_Ehdr *)elf_data;

    if (_memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0 || elf_header->e_ident[EI_CLASS] != ELFCLASS64 || elf_header->e_type != ET_EXEC) {
        return SYMBOL_NOT_FOUND; // Not a valid ELF64 executable
    }

    const Elf64_Shdr *section_headers = (const Elf64_Shdr *)(elf_data + elf_header->e_shoff);
    const char *strtab = NULL;

    // Find the section containing symbol names (usually .dynstr or .strtab)
    for (int i = 0; i < elf_header->e_shnum; ++i) {
        if (section_headers[i].sh_type == SHT_STRTAB) {
            strtab = (const char *)(elf_data + section_headers[i].sh_offset);
            break;
        }
    }

    if (!strtab) {
        return SYMBOL_NOT_FOUND; // No string table found
    }

    // Find the section containing the symbol table (usually .dynsym or .symtab)
    const Elf64_Shdr *symtab_header = NULL;
    for (int i = 0; i < elf_header->e_shnum; ++i) {
        if (section_headers[i].sh_type == SHT_SYMTAB) {
            symtab_header = &section_headers[i];
            break;
        }
    }

    if (!symtab_header) {
        return SYMBOL_NOT_FOUND; // No symbol table found
    }

    const Elf64_Sym *symtab = (const Elf64_Sym *)(elf_data + symtab_header->sh_offset);

    // Find the symbol by hash
    for (int i = 0; i < symtab_header->sh_size / sizeof(Elf64_Sym); ++i) {
        const char *symbol_name = strtab + symtab[i].st_name;
        if (fnv1a_32(symbol_name) == hash) {
            return (uint64_t)symtab[i].st_value;
        }
    }

    return SYMBOL_NOT_FOUND; // Symbol not found
}
