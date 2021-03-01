bits 64

section .start
    global _start
    extern initialize_standard_library
    extern main

bootinfo dq 0

_start:
    pop qword [bootinfo]    ; preserve bootinfo parameter

    call initialize_standard_library

    mov rdi, [bootinfo]
    call main

.hang:
    cli
    hlt
    jmp .hang