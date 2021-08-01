section .text
    global load
    extern main

SELECTOR_KERNEL_CODE    equ 0x08
SELECTOR_KERNEL_DATA    equ 0x10
KERNEL_ENTRY            equ 0x00101000

load:
bits 16
.init:
    mov word [.partition_offset], si    ; Save partition table offset
    mov byte [.boot_drive], dl          ; Save drive number

    cli  ; Disable interrupts

    ; Clear all general purpose registers.
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    xor esi, esi
    xor edi, edi
    xor ebp, ebp

    ; Initialize all segment registers to zero.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; create stack
    mov ss, ax
    mov sp, Mem.Loader.Stack.Top

.loadGDT32:
    lgdt [GDT32.Table.Pointer]

.enable_pmode:
    mov eax, cr0
    or eax, (1 << 0)    ; CR0.PE
    mov cr0, eax

    jmp GDT32.Selector.Code32 : .pmode

bits 32
.pmode:
    ; prepare parameters on the stack
    xor eax, eax
    mov ax, word [.partition_offset]
    push eax
    
    xor eax, eax
    mov al, byte [.boot_drive]
    push eax

    ; Initialize all data segment registers with the 32-bit protected mode
    ; data segment selector.
    mov ax, GDT32.Selector.Data32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; call c entry point
    cld
    call main

.hang:
    cli
    hlt
    jmp .hang

.partition_offset:  dw 0x0000
.boot_drive:        db 0x00

global lmode_enable
lmode_enable:
    ; Enable PAE, PSE, PGE paging.
    mov eax, cr4
    or eax, (1 << 5) | (1 << 4) | (1 << 7)  ; CR4.PAE | CR4.PSE | CR4.PGE
    mov cr4, eax

    ; Enable 64-bit mode mode and syscall/sysret.
    mov ecx, 0xc0000080 ; Extended Feature Enable Register (EFER)
    rdmsr
    or eax, (1 << 8) | (1 << 0)
    wrmsr

    ; Enable paging and protected mode.
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)    ; CR0.PG, CR0.PE
    mov cr0, eax

    ; Do a long jump using the new GDT, which forces the switch to 64-bit mode.
    jmp SELECTOR_KERNEL_CODE : .lmode

bits 64
.lmode:
    mov ax, SELECTOR_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rdi, rdi
    xor rsi, rsi
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

extern low_mem_size
    mov rsp, qword [low_mem_size]

extern bootinfo
    push bootinfo

    jmp KERNEL_ENTRY

%include "../include/memory_layout.inc"
%include "../include/gdt.inc"