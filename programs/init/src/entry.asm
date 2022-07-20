section .text
    global _start

_start:
    mov rax, 0
.loop:
    inc rax
    jmp .loop
