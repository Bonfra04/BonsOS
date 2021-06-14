OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    . = 0x8000000000;

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
