bits 64

%define EX_NMI  0x02
%define EX_DF   0x08
%define EX_TS   0x0A
%define EX_NP   0x0B
%define EX_SS   0x0C
%define EX_GP   0x0D
%define EX_PF   0x0E
%define EX_MC   0x12

section .text
    global isr_dispatcher
    global isr_set
    extern kernel_paging

isr_handlers: times 256 dq 0

; rdi interrupt, rsi function pointer
isr_set:
    mov [isr_handlers + rdi * 8], rsi
    ret

isr_dispatcher:
    ; if the interrupt has an error code, skip this part
    cmp byte [rsp], EX_NMI
    je .isr_error
    cmp byte [rsp], EX_DF
    je .isr_error
    cmp byte [rsp], EX_TS
    je .isr_error
    cmp byte [rsp], EX_NP
    je .isr_error
    cmp byte [rsp], EX_SS
    je .isr_error
    cmp byte [rsp], EX_GP
    je .isr_error
    cmp byte [rsp], EX_PF
    je .isr_error
    cmp byte [rsp], EX_MC
    je .isr_error

    ; push dummy error code
    push 0x0

    ; jump to the isr
    jmp .isr

.isr_error:
    ; use r14 and r15 to swap the interrupt number and the error code on the stack
    push r15
    push r14
    mov r14, [rsp + 8 * 2]  ; interrupt
    mov r15, [rsp + 8 * 3]  ; error code
    mov [rsp + 8 * 2], r15
    mov [rsp + 8 * 3], r14
    pop r14
    pop r15

.isr:

.push_context:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rax, gs
    push rax
    mov rax, fs
    push rax
    mov rax, es
    push rax
    mov rax, ds
    push rax

    mov r15, cr3                ; save cr3
    mov rax, [ kernel_paging ]  ; set kernel paging
    mov cr3, rax

.call_handler:
    ; find the handler
    mov rax, [rsp + 8 * 20]             ; get the interrupt number
    mov rax, [isr_handlers + 8 * rax]   ; get the handler
    cmp rax, 0
    je .pop_context

    ; call the handler
    cld
    lea rdi, [rsp]
    call rax

.pop_context:

    mov cr3, r15                ; restore cr3

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

.return:
    add rsp, 16      ; chop error code and interrupt #
    iretq
