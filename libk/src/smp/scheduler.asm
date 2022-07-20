bits 64

section .text
    global scheduler_tick
    global scheduler_replace_switch
    extern lapic_eoi
    extern get_next_thread
    extern current_thread

struc process_t
    .paging:         resq 1 
    .n_threads:      resq 1
endstruc

struc thread_t
    .stack_pointer:  resq 1
    .stack_base:     resq 1
    .process:        resq 1

    .next_thread:    resq 1
    .prev_thread:    resq 1
endstruc

scheduler_tick:
    pop rax                                     ; chop return address to isr_dispatcher

    mov rax, [ current_thread ]                 ; get current thread
    mov [ rax + thread_t.stack_pointer ], rsp   ; save current rsp

    call get_next_thread                        ; get next thread
    mov [ current_thread ], rax                 ; set current thread to next thread

    push rax
    call lapic_eoi                              ; send eoi to the lapic
    pop rdi
scheduler_replace_switch:
    mov rbx, [ rdi + thread_t.process ]         ; get thread's process
    mov rsp, [ rdi + thread_t.stack_pointer ]   ; set rsp
    mov rbx, [ rbx + process_t.paging ]         ; set paging ad5dr
    mov cr3, rbx

    ; restore the interrupt context
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

    add rsp, 2*8    ; remove error code and interrupt #

    iretq
