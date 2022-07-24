bits 64

%ifndef KERNEL_BUILD

section .text
    global _start
    extern main
    extern call_ctors
    extern exit

_start:
    push rdi ; save argc
    push rsi ; save argv
    call call_ctors
    pop rsi ; restore argv
    pop rdi ; restore argc
    call main
    mov rax, rdi ; exit status
    call exit

%endif
