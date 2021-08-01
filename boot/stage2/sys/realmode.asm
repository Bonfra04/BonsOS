section .text
    global rm_int

GDT32.Selector.Code32       equ     0x08    ; 32-bit protected mode (code)
GDT32.Selector.Data32       equ     0x10    ; 32-bit protected mode (data)
GDT32.Selector.Code16       equ     0x18    ; 16-bit protected mode (code)
GDT32.Selector.Data16       equ     0x20    ; 16-bit protected mode (data)

bits 32
rm_int:
    ; self-modifying code
    mov al, byte [esp + 4]
    mov byte [.int_no], al

    ; save regs_in
    mov eax, dword [esp + 8]
    mov dword [.regs_in], eax

    ; save regs_out
    mov eax, dword [esp + 12]
    mov dword [.regs_out], eax
    
    pushad

.prepareProtected16Mode:
    ; Before we can switch back to real mode, we have to switch to
    ; 16-bit protected mode.
    jmp GDT32.Selector.Code16 : .switchToProtected16Mode

bits 16
.switchToProtected16Mode:
    ; Initialize all data segment registers with the 16-bit protected mode
    ; data segment selector.
    mov ax, GDT32.Selector.Data16
    mov ds, ax
    mov es, ax
    mov ss, ax

.prepareRealMode:
    ; Disable protected mode.
    mov eax, cr0
    and eax, ~(1 << 0)   ; CR0.PE
    mov cr0, eax

    ; Do a far jump to switch back to real mode.
    jmp 0 : .switchToRealMode

.switchToRealMode:
    ; Restore real mode data segment registers.
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

.interrupt:
    ; load regs_in
    mov dword [.esp], esp
    mov esp, dword [.regs_in]
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    mov esp, dword [.esp]

    sti

    ; Indirect interrupt call
    db 0xcd
    .int_no: db 0

    cli

    ; load out regs_out
    mov dword [ss:.esp], esp
    mov esp, dword [ss:.regs_out]
    lea esp, [esp + 7 * 4]
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov esp, dword [ss:.esp]

.prepareProtected32Mode:
    ; Enable protected mode.
    mov eax, cr0
    or eax, (1 << 0)    ; CR.PE
    mov cr0, eax

    ; Do a far jump to switch to 32-bit protected mode.
    jmp GDT32.Selector.Code32 : .switchToProtected32Mode

bits 32
.switchToProtected32Mode:
    ; Initialize all data segment registers with the 32-bit protected mode
    ; data segment selector.
    mov ax, GDT32.Selector.Data32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    popad
    ret

section .bss
align 16
    .esp:       resd 1
    .regs_in:   resd 1
    .regs_out:  resd 1