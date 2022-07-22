section .text
    global _start

_start:
    mov rax, 0
.loop:
    syscall
    inc rax
    jmp .loop
