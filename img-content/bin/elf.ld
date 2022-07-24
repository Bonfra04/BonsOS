OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    . = 0x8000000000;

    .text : ALIGN(0x1000)
    {
        *(.start)
        *(.text)
    }

    .data : ALIGN(0x1000)
    {
        *(.data)
    }

    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }

    .ctors : ALIGN(0x1000)
    {
        __start_ctors = .;
        *(.ctors)
        __end_ctors = .;
    }

    .dtors : ALIGN(0x1000)
    {
        __start_dtors = .;
        *(.dtors)
        __end_dtors = .;
    }

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
}
