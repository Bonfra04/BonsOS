bits 64

SELECTOR_KERNEL_DATA equ 0x08
SELECTOR_KERNEL_CODE equ 0x10

SELECTOR_USER_DATA equ 0x18
SELECTOR_USER_CODE equ 0x20

PRIVILEGE_KERNEL equ 0
PRIVILEGE_USER equ 1

section .text
    global schedule_isr
    extern set_thread_stack
    extern get_next_thread

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

    ; cr3
    mov rax, cr3
    mov [.r_cr3], rax

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
    
    push qword [.r_cr3]

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

; thread_t* get_next_thread()
.select_next_thread:
    call get_next_thread ; rax contains a pointer to the next thread

.restore_segments:
    mov rbx, qword [rax + 5 * 8] ; rbx points to parent process
    mov ecx, dword [rbx + 1 * 8] ; ecx contains the privilege
    mov [.priv], ecx

    cmp dword [.priv], PRIVILEGE_KERNEL
    je .kernel_seg
.user_seg:
    mov rbx, SELECTOR_USER_DATA | 3
    mov ds, rbx
    mov rbx, SELECTOR_USER_DATA | 3
    mov es, rbx
    mov rbx, SELECTOR_USER_DATA | 3
    mov fs, rbx
    mov rbx, SELECTOR_USER_DATA | 3
    mov gs, rbx
    jmp .seg_done
.kernel_seg:
    mov rbx, SELECTOR_KERNEL_DATA
    mov ds, rbx
    mov rbx, SELECTOR_KERNEL_DATA
    mov es, rbx
    mov rbx, SELECTOR_KERNEL_DATA
    mov fs, rbx
    mov rbx, SELECTOR_KERNEL_DATA
    mov gs, rbx
.seg_done:

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

    add rsp, 1 * 8 ; thrash cr3 for now

    pop qword [.r_flags]

.prepare_ret_stack:
    cmp dword [.priv], PRIVILEGE_KERNEL
    je .kernel_stack
.user_stack:
    push SELECTOR_USER_DATA | 3 ; ss

    ;rsp
    mov [.r_rax], rax
    mov rax, rsp
    add rax, 8
    push rax
    mov rax, [.r_rax]

    push qword [.r_flags]   ; rflags
    push SELECTOR_USER_CODE | 3 ; cs
    push qword [.r_rip]     ; rip
    jmp .stack_done
.kernel_stack:
    push SELECTOR_KERNEL_DATA ; ss
    
    ;rsp
    mov [.r_rax], rax
    mov rax, rsp
    add rax, 8
    push rax
    mov rax, [.r_rax]

    push qword [.r_flags]   ; rflags
    push SELECTOR_KERNEL_CODE ; cs
    push qword [.r_rip]     ; rip
.stack_done:

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

.r_cr3: resq 1

.r_flags: resq 1

.r_rip: resq 1

.r_rsp: resq 1
