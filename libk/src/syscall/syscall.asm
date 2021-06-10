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
    call tss_get_kstack     ; rax contains virtual address of kstack top
    sub rax, KSTACK_SIZE    ; rax contains virtual address of kstack base
    mov rsi, rax
    call paging_get_ph      ; rax contains physical kstack rsp
    mov rsp, rax            ; set new stack

    ; enable back interrupts
    sti

.call_handler:
    ; set parameters
    push 10 ; r15
    push 20 ; r14
    push 30 ; r13
    push 40 ; r12
    push 50 ; r10
    push 60 ; r9
    push 70 ; r8

    ; get function address
    mov rax, [.syscall_id]
    mov rax, [syscalls + 8 * rax]   ; rax=ISR address

    ; System V ABI requires direction flag to be cleared on function entry.
    cld

    ; set parameter
    mov rdi, rsp

    call rax

    ; pop parameters
    add rsp, 8 * 7

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

    mov [.r_rax], rax
    pop rax
    mov cr3, rax
    mov rax, [.r_rax]

    mov rsp, [.r_rsp]
    o64 sysret

section .bss

.syscall_id: resq 1

.r_rsp: resq 1
.r_rax: resq 1

.tmp_rsp: resq 1
.tmp_stack: resq 256
.tmp_stack_top:
