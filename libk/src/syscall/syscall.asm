bits 64

section .text
    global syscall_handle
    extern syscall_handlers
    extern kernel_paging

struc gdtr_t
    .size:      resw 1
    .offset:    resq 1
endstruc

struc gdt_t
    .gdt_descs: resq 5
    .tss_desc:  resq 2
endstruc

struc tss_desc_t
    .limit_low:         resw 1
    .base_low:          resw 1
    .base_middle:       resb 1
    .access:            resb 1
    .limit_high_flags:  resb 1
    .base_high:         resb 1
    .base_highest:      resd 1
    .reserved:          resd 1
endstruc

SELECTOR_KERNEL_DATA equ 0x10
SELECTOR_USER_DATA equ 0x18

MAX_SYSCALLS equ 5

syscall_handle:
    push r11        ; save flags
    push rcx        ; save ret addr
    mov r10, cr3    ; save paging
    mov r11, rsp    ; save rsp
    ; rax contains syscall number

    ; retrive gdt address
    sub rsp, 8+2
    sgdt [ rsp ]
    mov r8, [ rsp + gdtr_t.offset ]

    ; use kernel paging
    mov r9, [ kernel_paging ]
    mov cr3, r9

    ; get tss entry address
    mov ecx, [ r8 + gdt_t.tss_desc + tss_desc_t.base_highest ]
    shl rcx, 32
    mov cx, [ r8 + gdt_t.tss_desc + tss_desc_t.base_high ]
    shl cx, 8
    mov cl, [ r8 + gdt_t.tss_desc + tss_desc_t.base_middle ]
    shl ecx, 16
    mov cx, [ r8 + gdt_t.tss_desc + tss_desc_t.base_low ]

    ; get stack to kernel stack
    mov rsp, [ rcx + 4 ]

    ; set segments
    mov r8, SELECTOR_KERNEL_DATA
    mov ds, r8
    mov es, r8
    mov fs, r8
    mov gs, r8

    ; save preserved registers
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; enable interrupts
    sti

    ; check if syscall number is valid
    cmp rax, MAX_SYSCALLS
    jge .restore_context
    ; save syscall
    mov rax, [ syscall_handlers + rax * 8 ]

    ; call syscall
    cld     ; System V ABI requires direction flag to be cleared on function entry
    call rax

.restore_context:
    ; disable interrupts
    cli

    ; restore preserved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    ; restore segments
    mov r8, SELECTOR_USER_DATA | 3
    mov ds, r8
    mov es, r8
    mov fs, r8
    mov gs, r8

    mov rsp, r11    ; restore rsp
    mov cr3, r10    ; restore paging
    pop rcx         ; restore ret addr
    pop r11         ; restore flags
    o64 sysret
