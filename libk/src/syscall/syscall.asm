bits 64

section .text
    global syscall_handle
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

syscall_handle:
    mov r10, rsp    ; save rsp
    mov r9, rcx     ; save return addr
    mov r8, r11     ; save flags
    mov rdx, cr3    ; save cr3

    ; retrive gdt address
    sub rsp, 8+2
    sgdt [ rsp ]
    mov rdi, [ rsp + gdtr_t.offset ]

    ; use kernel paging
    mov rdx, [ kernel_paging ]
    mov cr3, rdx

    ; get tss entry address
    mov ecx, [ rdi + gdt_t.tss_desc + tss_desc_t.base_highest ]
    shl rcx, 32
    mov ch, [ rdi + gdt_t.tss_desc + tss_desc_t.base_high ]
    mov cl, [ rdi + gdt_t.tss_desc + tss_desc_t.base_middle ]
    shl ecx, 16
    mov cx, [ rdi + gdt_t.tss_desc + tss_desc_t.base_low ]

    ; get stack to kernel stack
    mov rsp, [ rcx + 4 ]

    ; set segments
    mov rcx, SELECTOR_KERNEL_DATA
    mov ds, rcx
    mov es, rcx
    mov fs, rcx
    mov gs, rcx

    ; save preserved registers
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    
    ; enable interrupts
    sti

    ; dispatcher code here

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
    mov rcx, SELECTOR_USER_DATA | 3
    mov ds, rcx
    mov es, rcx
    mov fs, rcx
    mov gs, rcx

    mov rsp, r10    ; restore rsp
    mov rcx, r9     ; restore return addr
    mov r11, r8     ; restore flags
    mov cr3, rdx    ; restore cr3
    o64 sysret