bits 64

%ifndef KERNEL_BUILD

section .text
    global _start
    extern main

_start:
    call main
    jmp $

%endif
