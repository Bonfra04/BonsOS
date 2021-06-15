bits 64

section .text
    global _start
    extern main

_start:
    call main       ; main()
    mov rax, 1      ; thread_terminate(
    syscall         ; )

.hang:
    pause
    jmp .hang
