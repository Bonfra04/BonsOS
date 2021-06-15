section .text
    global sys_fwrite

sys_fwrite:
    push r8
    push r9
    push r12
    push rax

    mov r8, rdi
    mov r9, rsi
    mov r12, rdx
    mov rax, 4
    syscall

    pop rax
    pop r12
    pop r9
    pop r8

    ret