section .text
    global setjmp
    global longjmp

setjmp:
    mov [rdi + 0x00], rbx
    mov [rdi + 0x08], rbp
    mov [rdi + 0x10], r12
    mov [rdi + 0x18], r13
    mov [rdi + 0x20], r14
    mov [rdi + 0x28], r15

    lea rax, [rsp + 8]      ; rsp before return rip is pushed

    mov [rdi + 0x30], rax
    mov rax, [rsp]          ; return rip
    mov [rdi + 0x38], rax

    mov rax, 0
    ret

longjmp:
    mov rbx, [rdi + 0x00]
    mov rbp, [rdi + 0x08]
    mov r12, [rdi + 0x10]
    mov r13, [rdi + 0x18]
    mov r14, [rdi + 0x20]
    mov r15, [rdi + 0x28]

    mov rax, rsi
    mov rsp, [rdi + 0x30]
    jmp [rdi + 0x38]