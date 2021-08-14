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
    extern scheduler_get_current_paging

; save context before calling, nothing is granted to be preserved
syscall_handle:
    mov rbp, rsp    ; save rsp
    mov rbx, rax    ; save syscall id
    mov r10, rcx    ; save return addr
    ; preserve rcx(retaddr) and r11(flags)

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

.save_context:
    push r10    ; return addr
    push r11    ; flags

.call_handler:
    ; get function address
    mov rax, rbx
    cmp rax, 32 ; temporary max syscall id
    jg .restore_context
    mov rax, [syscalls + 8 * rax]   ; rax=ISR address
    
    sti

    ; set parameters
    push r12
    push r13
    push r14
    push r15

    ; System V ABI requires direction flag to be cleared on function entry.
    cld

    ; set parameter
    mov rdi, rsp

    ; call handler
    call rax
    mov rbx, rax    ; save return value

    ; pop parameters
    add rsp, 8 * 4 ; 4 qwords
    
.restore_context:
    cli

    pop r11     ; flags
    pop r10     ; return addr

    ; exit syscall mode
    call scheduler_toggle_syscall_state 

    ; switch to process paging
    call scheduler_get_current_paging ; qua si sminchia rcx return address per sysret
    mov cr3, rax

    ; restore rsp
    mov rsp, rbp

    ; restore segments
    mov rax, SELECTOR_USER_DATA | 3
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax

    mov rax, rbx ; set return value
    mov rcx, r10
    o64 sysret

section .bss

.return_val: resq 1

.tmp_stack: resq 32
.tmp_stack_top:
