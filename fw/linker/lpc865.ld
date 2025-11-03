MEMORY {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
    RAM (rwx)  : ORIGIN = 0x10000000, LENGTH = 8K
}

SECTIONS {
    /* Place vector table at the beginning of flash */
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH

    /* Fill gap with 0xFF until 0x00000400 */
    .gap : {
        /* Align to 0x400 */
        . = ALIGN(4);
        FILL(0xFF)
        . = 0x00000400;
    } > FLASH

    /* User code starts at second sector */
    .text : {
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data)) ;
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

        *(.text*)
        *(.rodata*)
    } > FLASH

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > RAM
}

/* Stack placement */
PROVIDE(__stack = ORIGIN(RAM) + LENGTH(RAM));
PROVIDE(__stacklimit = __stack - 0x800);    /* reserve 2k for stack */
