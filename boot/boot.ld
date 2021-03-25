OUTPUT_FORMAT(binary)
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
