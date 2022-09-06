bits 64

%ifndef KERNEL_BUILD

section .text
    global _start
    extern main
    extern __libc_init
    extern call_ctors
    extern call_dtors
    extern exit

_start:
    push rdi    ; save argc
    push rsi    ; save argv

    mov rdi, rdx
    call __libc_init
    call call_ctors

    pop rsi     ; restore argv
    pop rdi     ; restore argc
    call main
    push rax    ; save exit code
    call call_dtors
    pop rdi     ; restore exit code
    call exit

%endif
