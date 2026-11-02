OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = RELOC_ADDR;     /* VMA: execute here, but the code is placed at 0x80200000 */
    
    .text : AT(LOAD_ADDR) {     /* LMA: code is placed here */
        KEEP(*(.text.boot))     /* avoid .text.boot from deletion by linker even if the section is not used and ensure .text.boot will be exec. first */
        *(.text .text.*)        /* place all code section from other obj file to here(text seg.) */
    }
    .rodata : { *(.rodata .rodata.* .srodata .srodata.*) }
    .data : { *(.data .data.* .sdata .sdata.*) }
    
    . = ALIGN(8);
    __reloc_image_end = .;      /* relocate from "_start" to here */

    .bss (NOLOAD) : {       /* no need to load the initialization contents */
        __bss_start = .;
        *(.sbss .sbss.* .bss .bss.* COMMON)
        . = ALIGN(8);
        __bss_stop = .;
    }
    . = ALIGN(16);      /* stack pointer should be 16 bytes alignment */
    . = . + 0x4000;     /* 16KB for stack */
    _end = .;
}
