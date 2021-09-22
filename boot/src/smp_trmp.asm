org 0x8000

bits 16

smp_trampoline:
    jmp boot
    kernel_paging: dd 0x00
    entry_point: dq 0x00

boot:
    cli
    cld

.init:
    ; clear all general purpose registers
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    xor esi, esi
    xor edi, edi
    xor ebp, ebp

    ; initialize all segment registers to zero
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; create stack
    mov esp, 0x8000

.enable_sse:
    call ActivateSSE

.load_gdt:
    lgdt [GDT64.Table.Pointer]

.setup_paging:
    ; set paging hierarchy address
    mov eax, dword [kernel_paging]
    mov cr3, eax
    ; enable paging
    mov eax, cr4
    or eax, (1 << 5) | (1 << 4) | (1 << 7)  ; CR4.PAE | CR4.PSE | CR4.PGE
    mov cr4, eax

.prepare_long_mode:

    ; enable 64-bit mode mode and syscall/sysret.
    mov ecx, 0xc0000080 ; extended feature enable register (EFER)
    rdmsr
    or eax, (1 << 8) | (1 << 0)
    wrmsr

    ; enable paging and protected mode.
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)    ; CR0.PG, CR0.PE
    mov cr0, eax

    jmp GDT64.Selector.Kernel.Code : .long_mode

bits 64

.long_mode:
    ; set up the data segment registers.
    mov ax, GDT64.Selector.Kernel.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; initialize all general purpose registers.
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

    ; signal the bsp that the core has been booted succesfully
    jmp qword [entry_point]

%include "src/gdt.inc"
%include "src/sse.inc"