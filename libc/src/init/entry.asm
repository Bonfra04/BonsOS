bits 64

%ifndef KERNEL_BUILD

section .text
    global _start
    extern main
    extern call_ctors
    extern exit

_start:
    call call_ctors
    call main
    call exit

%endif
