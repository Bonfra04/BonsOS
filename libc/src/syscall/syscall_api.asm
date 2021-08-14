section .text
    global sys

sys:
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, rdi    ; syscall id
    mov r15, rsi    ; 1st param
    mov r14, rdx    ; 2nd param
    mov r13, rcx    ; 3rd param
    mov r12, r8     ; 4th param

    syscall

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdi
    pop rcx
    pop rdx
    pop rbx

    ret