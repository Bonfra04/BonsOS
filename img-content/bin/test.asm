bits 64

section .text
    global _start

_start:
    mov rax, 4      ; write_file(
    mov r8, 1       ;   STD_OUT,
    mov r9, str     ;   str,
    mov r10, str_len;   str_len,
    syscall         ; );
    jmp _start

section .data
    str: db "Hello, World!", 10, 0
    str_len equ $ - str