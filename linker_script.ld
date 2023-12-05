ENTRY(_start);

SECTIONS {
    . = 0x0400200;

    .text : {
        *(.text.startup)
        *(.text.*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    .rodata : {
        *(.rodata)
    }

    .symtab : {
        *(.symtab)
    }

    .strtab : {
        *(.strtab)
    }

    .shstrtab : {
        *(.shstrtab)
    }
}
