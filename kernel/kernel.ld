OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    . = 0x00100000 + 0x1000;

    __kernel_start_addr = .;

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
        __bss_start_addr = .;
        *(COMMON)
        *(.bss)
        __bss_size = . - __bss_start_addr;
    }

    __kernel_end_addr = .;
}
