bits 64

section .text
    global restore_context

restore_context:
    pop rax
    mov ds, rax
    pop rax
    mov es, rax
    pop rax
    mov fs, rax
    pop rax
    mov gs, rax

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    pop qword [.r_ss]
    pop qword [.r_cs]
    pop qword [.r_flags]
    pop qword [.r_rip]

.prepare_ret_stack:
    push qword [.r_ss]

    ;rsp
    mov [.r_rax], rax
    mov rax, rsp
    add rax, 8
    push rax
    mov rax, [.r_rax]

    push qword [.r_flags]   ; rflags
    push qword [.r_cs]
    push qword [.r_rip]     ; rip

.interrupt_done:
    push rax    ; save rax
    
    ; slave
    mov al, 0x20    ; PIC_CMD_EOI
    out 0xA0, al    ; PIC_CMD_SLAVE
    ; master
    mov al, 0x20    ; PIC_CMD_EOI
    out 0x20, al    ; PIC_CMD_MASTER
    
    pop rax     ; restore rax

.done:
    iretq

section .bss

.r_rax: resq 1
.r_ss: resq 1
.r_cs: resq 1
.r_flags: resq 1
.r_rip: resq 1
