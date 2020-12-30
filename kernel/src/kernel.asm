bits 64

section .start
    global _start
    extern main

_start:
    pop rdi     ; bootinfo parameter
    call main

.hang:
    cli
    hlt
    jmp .hang