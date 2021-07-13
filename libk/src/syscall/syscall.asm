bits 64

SELECTOR_KERNEL_DATA equ 0x10
SELECTOR_USER_DATA equ 0x18
KSTACK_SIZE equ 0x1000

section .text
    global syscall_handle
    extern syscalls
    extern kernel_paging
    extern tss_get_kstack
    extern paging_get_ph
    extern scheduler_toggle_syscall_state
    extern get_current_process_paging

; screw up: r15, r14, r11, rax, rbx, rcx 
syscall_handle:
    mov r15, rsp    ; save rsp
    mov r14, rax    ; save syscall id

.switch_context:

    ; set kernel paging
    mov rdi, cr3    ; save process paging for later use
    mov rax, [kernel_paging]
    mov cr3, rax

    ; set segments
    mov rax, SELECTOR_KERNEL_DATA
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax

    ; switch to kernel stack
    mov rsp, .tmp_stack_top
    call tss_get_kstack
    sub rax, KSTACK_SIZE
    mov rsi, rax
    ; rdi already contains the process paging
    call paging_get_ph      ; rax contains physical kstack base
    add rax, KSTACK_SIZE    ; rax contains physical kstack top
    mov rsp, rax            ; set new stack
    
    ; enter syscall mode
    call scheduler_toggle_syscall_state

    push rax
    push rbx
    push rcx    ; return addr
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11    ; flags
    push r12
    push r13
    push r14
    push r15

.call_handler:
    ; get function address
    mov rax, r14
    cmp rax, 32 ; temporary max syscall id
    jg .restore_context
    mov rax, [syscalls + 8 * rax]   ; rax=ISR address
    
    sti

    ; set parameters
    push r13 ; r13
    push r12 ; r12
    push r10 ; r10
    push r9  ; r9
    push r8  ; r8

    ; System V ABI requires direction flag to be cleared on function entry.
    cld

    ; set parameter
    mov rdi, rsp

    ; call handler
    call rax
    mov rbx, rax

    ; pop parameters
    add rsp, 8 * 5 ; 5 qwords
    
.restore_context:
    cli

    ; exit syscall mode
    call scheduler_toggle_syscall_state 

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11     ; flags 
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx     ; return addr
    ;pop rbx    ; used for return value
    pop rax

    ; switch to process paging
    call get_current_process_paging
    mov cr3, rax

    ; restore rsp
    mov rsp, r15

    ; restore segments
    mov rax, SELECTOR_USER_DATA
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax

    mov rax, rbx ; set return value
    o64 sysret

section .bss

.return_val: resq 1

.tmp_stack: resq 32
.tmp_stack_top:
