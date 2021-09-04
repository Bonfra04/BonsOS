OUTPUT_FORMAT(elf32-i386)
ENTRY(load)

SECTIONS
{
    . = 0x7E00;

    .text :
    {
        *(.start)
        *(.text)
    }

    .data :
    {
        *(.data)
    }

    .rodata :
    {
        *(.rodata)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}
