bits 64

section .text
    global _start
    extern main

_start:
    call main       ; main()
    mov rax, 1      ; process_terminate(
    syscall         ; )

.hang:
    pause
    jmp .hang
