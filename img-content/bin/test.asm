org 0x8000000000

bits 64

section .text

_main:
    mov rax, 4      ; write_file(
    mov r8, 1       ;   STD_OUT,
    mov r9, str     ;   str,
    mov r10, str_len;   str_len,
    syscall         ; );
    jmp _main

section .data
    str: db "Hello, World", 0
    str_len equ $ - str