section .text
    global sys

sys:
    push r8
    push r9
    push r10
    push r11
    push r12

    mov r12, r9     ; 6th param
    mov r11, r8     ; 5th param
    mov r10, rcx    ; 4th param
    mov r9, rdx     ; 3rd param
    mov r8, rsi     ; 2nd param
    mov rax, rdi    ; 1st param

    syscall

    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    ret