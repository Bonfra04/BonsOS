bits 64

section .start
    global _start
    extern main

_start:
    pop qword [bootinfo]    ; preserve bootinfo parameter

    mov rdi, [bootinfo]
    call main

.hang:
    cli
    hlt
    jmp .hang

section .bss
    bootinfo resq 1