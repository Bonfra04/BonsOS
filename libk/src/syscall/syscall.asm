bits 64

SELECTOR_KERNEL_DATA equ 0x10
KSTACK_SIZE equ 0x1000

section .text
    global syscall_handle
    extern syscalls
    extern kernel_paging
    extern tss_get_kstack
    extern paging_get_ph

syscall_handle:
    mov [.r_rsp], rsp       ; preserve stack
    mov rsp, .tmp_stack_top ; set temporary stack

.save_id:
    mov [.syscall_id], rax

.preserve_context:
    mov rax, cr3
    push rax

    mov rax, [.syscall_id]

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

    mov rax, ds
    push rax
    mov rax, es
    push rax
    mov rax, fs
    push rax
    mov rax, gs
    push rax

    ; save tmp stack rsp
    mov [.tmp_rsp], rsp

.init_kernel_state:
    ; set segments
    mov rax, SELECTOR_KERNEL_DATA
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax

    ; set kernel paging
    mov rax, cr3            ; save old paging
    mov rbx, [kernel_paging]
    mov cr3, rbx            ; set new paging
    
    ; set kernel stack
    mov rdi, rax            ; old paging
    call tss_get_kstack     ; rax contains virtual kstack top
    sub rax, KSTACK_SIZE    ; rax contains virtual kstack base
    mov rsi, rax
    call paging_get_ph      ; rax contains physical kstack base
    add rax, KSTACK_SIZE    ; rax contains physical kstack top
    mov rsp, rax            ; set new stack

    ; enable back interrupts
    sti

.call_handler:
    ; set parameters
    push r15 ; r15
    push r14 ; r14
    push r13 ; r13
    push r12 ; r12
    push r11 ; r11
    push r10 ; r10
    push r9  ; r9
    push r8  ; r8

    ; get function address
    mov rax, [.syscall_id]
    mov rax, [syscalls + 8 * rax]   ; rax=ISR address

    ; System V ABI requires direction flag to be cleared on function entry.
    cld

    ; set parameter
    mov rdi, rsp

    ; call handler
    call rax
    mov [.return_val], rax

    ; pop parameters
    add rsp, 8 * 8

    ; disable interrupts
    cli

.restore_context:
    ; set tmp rsp
    mov rsp, [.tmp_rsp]

    pop rax
    mov gs, rax
    pop rax
    mov fs, rax
    pop rax
    mov es, rax
    pop rax
    mov ds, rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    pop rax
    mov cr3, rax
    mov rax, [.return_val]

    mov rsp, [.r_rsp]
    o64 sysret

section .bss

.syscall_id: resq 1
.return_val: resq 1

.r_rsp: resq 1
.r_rax: resq 1

.tmp_rsp: resq 1
.tmp_stack: resq 512
.tmp_stack_top:
