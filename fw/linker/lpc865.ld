MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
    RAM (rwx)  : ORIGIN = 0x10000000, LENGTH = 8K
}

SECTIONS
{
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
