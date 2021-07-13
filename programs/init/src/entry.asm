bits 64

section .text
    global _start
    extern main
    extern initialize_standard_library
    extern initialize_user_library

_start:
    ; preserve arguments
    push rdi
    push rsi

    call initialize_standard_library
    call initialize_user_library

    ; restore arguments
    pop rsi
    pop rdi

    call main       ; main()
    mov rax, 1      ; thread_terminate(
    syscall         ; )

.hang:
    pause
    jmp .hang
