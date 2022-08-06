bits 64

section .text
    global scheduler_tick
    global scheduler_replace_switch
    extern get_next_thread
    extern current_thread
    extern tss_set_kstack
    extern lapic_eoi
    extern kernel_paging

struc process_t
    .paging:         resq 1 
    .executable:     resq 1
    .resources:      resq 1
    .threads:        resq 1
endstruc

struc thread_t
    .stack_pointer:  resq 1
    .stack_base:     resq 1

    .kstack_pointer: resq 1
    .kstack_base:    resq 1 

    .process:        resq 1

    .next_thread:    resq 1
    .prev_thread:    resq 1
endstruc

struc interrupt_context_t
    .r_ds:           resq 1
    .r_es:           resq 1
    .r_fs:           resq 1
    .r_gs:           resq 1
    .r_rax:          resq 1
    .r_rbx:          resq 1
    .r_rcx:          resq 1
    .r_rdx:          resq 1
    .r_rsi:          resq 1
    .r_rdi:          resq 1
    .r_rbp:          resq 1
    .r_r8:           resq 1
    .r_r9:           resq 1
    .r_r10:          resq 1
    .r_r11:          resq 1
    .r_r12:          resq 1
    .r_r13:          resq 1
    .r_r14:          resq 1
    .r_r15:          resq 1
    .error:          resq 1
    .interrupt:      resq 1
    .retaddr:        resq 1
    .r_cs:           resq 1
    .r_flags:        resq 1
    .r_rsp:          resq 1
    .r_ss:           resq 1
endstruc

scheduler_tick:
    pop rax                                     ; chop return address to isr_dispatcher

    mov rax, [ rdi + interrupt_context_t.r_cs ] ; get code segment
    test rax, 0b11                              ; check if coming from kernel
    jnz .not_kernel                             
.kernel:                                        ; if so save kernel stack
    mov rax, [ current_thread ]
    mov [ rax + thread_t.kstack_pointer ], rsp  ; save kernel stack pointer
    call tss_set_kstack                         ; save kernel stack pointer
    jmp .continue
.not_kernel:                                    ; else enable kernel paging
    mov rax, [ kernel_paging ]                  ; sets back kernel paging
    mov cr3, rax

.continue:
    mov rax, [ current_thread ]                 ; get current thread
    mov [ rax + thread_t.stack_pointer ], rsp   ; save current rsp

    call get_next_thread                        ; get next thread
    mov [ current_thread ], rax                 ; set current thread to next thread

    push rax
    call lapic_eoi                              ; send eoi to the lapic
    pop rdi
scheduler_replace_switch:
    mov rbx, rdi                                ; copy argument address
    
    mov r15, [ rbx + thread_t.kstack_pointer ]  ; get thread kstack
    mov rdi, r15
    call tss_set_kstack                         ; set kstack in the address

    cmp r15, [ rbx + thread_t.stack_pointer ]   ; check if going to kernel
    jne .not_kernel
.kernel:                                        ; if so just restore krsp
    mov rsp, r15
    jmp .continue
.not_kernel:                                    ; else restore user rsp and paging
    mov rsp, [ rbx + thread_t.stack_pointer ]
    mov rbx, [ rbx + thread_t.process ]         ; get thread's process
    mov rbx, [ rbx + process_t.paging ]         ; get process's paging
    mov cr3, rbx                                ; set paging addr

.continue:

.restore_context:
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
