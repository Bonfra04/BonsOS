bits 64

section .text
    global _start
    extern main
    extern initialize_standard_library

_start:
    call initialize_standard_library

    call main       ; main()
    mov rax, 1      ; thread_terminate(
    syscall         ; )

.hang:
    pause
    jmp .hang
