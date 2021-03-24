bits 64

section .start
    global _start
    extern initialize_standard_library
    extern main

_start:
    pop qword [bootinfo]    ; preserve bootinfo parameter

    call initialize_standard_library

    mov rdi, [bootinfo]
    call main

.hang:
    cli
    hlt
    jmp .hang

section .bss
    bootinfo dq 0