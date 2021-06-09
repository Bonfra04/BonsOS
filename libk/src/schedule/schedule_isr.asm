bits 64

section .text
    global schedule_isr
    extern set_thread_stack
    extern get_next_thread
    extern set_thread_paging

;*****;
; Parameters:
;   rdi: pointer to interrupt_context
;*****;
schedule_isr:
.preserve_regs:
    ; rax
    mov rax, [rdi + 4 * 8]
    mov [.r_rax], rax
    ; rbx
    mov rax, [rdi + 5 * 8]
    mov [.r_rbx], rax
    ; rcx
    mov rax, [rdi + 6 * 8]
    mov [.r_rcx], rax
    ; rdx
    mov rax, [rdi + 7 * 8]
    mov [.r_rdx], rax
    ; rsi
    mov rax, [rdi + 8 * 8]
    mov [.r_rsi], rax
    ; rdi
    mov rax, [rdi + 9 * 8]
    mov [.r_rdi], rax
    ; rbp
    mov rax, [rdi + 10 * 8]
    mov [.r_rbp], rax
    ; r8
    mov rax, [rdi + 11 * 8]
    mov [.r_r8], rax
    ; r9
    mov rax, [rdi + 12 * 8]
    mov [.r_r9], rax
    ; r10
    mov rax, [rdi + 13 * 8]
    mov [.r_r10], rax
    ; r11
    mov rax, [rdi + 14 * 8]
    mov [.r_r11], rax
    ; r12
    mov rax, [rdi + 15 * 8]
    mov [.r_r12], rax
    ; r13
    mov rax, [rdi + 16 * 8]
    mov [.r_r13], rax
    ; r14
    mov rax, [rdi + 17 * 8]
    mov [.r_r14], rax
    ; r15
    mov rax, [rdi + 18 * 8]
    mov [.r_r15], rax

    ; ds
    mov rax, [rdi + 0 * 8]
    mov [.r_ds], rax
    ; es
    mov rax, [rdi + 1 * 8]
    mov [.r_es], rax
    ; fs
    mov rax, [rdi + 2 * 8]
    mov [.r_fs], rax
    ; gs
    mov rax, [rdi + 3 * 8]
    mov [.r_gs], rax
    ; ss
    mov rax, [rdi + 25 * 8]
    mov [.r_ss], rax
    ; cs
    mov rax, [rdi + 22 * 8]
    mov [.r_cs], rax

    ; flags
    mov rax, [rdi + 23 * 8]
    mov [.r_flags], rax

    ; rip
    mov rax, [rdi + 21 * 8]
    mov [.r_rip], rax

    ; rsp
    mov rax, [rdi + 24 * 8]
    mov [.r_rsp], rax

.save_regs:

    ; restore stack
    mov rsp, [.r_rsp]

    ; push registers
    push qword [.r_flags]

    push qword [.r_cs]
    push qword [.r_ss]

    push qword [.r_gs]
    push qword [.r_fs]
    push qword [.r_es]
    push qword [.r_ds]

    push qword [.r_r15]
    push qword [.r_r14]
    push qword [.r_r13]
    push qword [.r_r12]
    push qword [.r_r11]
    push qword [.r_r10]
    push qword [.r_r9]
    push qword [.r_r8]
    push qword [.r_rbp]
    push qword [.r_rdi]
    push qword [.r_rsi]
    push qword [.r_rdx]
    push qword [.r_rcx]
    push qword [.r_rbx]
    push qword [.r_rax]

    push qword [.r_rip]
    
; void set_thread_stack(size_t rsp)
.save_thread_stack:
    mov rdi, rsp
    call set_thread_stack

; switch to temporary stack
    mov rsp, .handler_stack_top

; void set_thread_paging(size_t cr3)
.save_thread_cr3:
    mov rdi, cr3
    call set_thread_paging

; thread_t* get_next_thread()
.select_next_thread:
    call get_next_thread ; rax contains a pointer to the next thread

.restore_registers:

    ; set other thread stack
    mov rsp, qword [rax + 0 * 8]

    ; rip
    pop rax
    mov [.r_rip], rax

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

    mov [.r_rax], rax
    pop rax
    mov ds, rax
    pop rax
    mov es, rax
    pop rax
    mov fs, rax
    pop rax
    mov gs, rax

    pop rax
    mov [.r_ss], rax
    pop rax
    mov [.r_cs], rax
    mov rax, [.r_rax]

    pop qword [.r_flags]

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

.priv: resd 1

.r_rax: resq 1
.r_rbx: resq 1
.r_rcx: resq 1
.r_rdx: resq 1
.r_rsi: resq 1
.r_rdi: resq 1
.r_rbp: resq 1
.r_r8: resq 1
.r_r9: resq 1
.r_r10: resq 1
.r_r11: resq 1
.r_r12: resq 1
.r_r13: resq 1
.r_r14: resq 1
.r_r15: resq 1

.r_ds: resq 1
.r_es: resq 1
.r_fs: resq 1
.r_gs: resq 1
.r_ss: resq 1
.r_cs: resq 1

.r_flags: resq 1

.r_rip: resq 1

.r_rsp: resq 1

.handler_stack_base: resq 64
.handler_stack_top:
