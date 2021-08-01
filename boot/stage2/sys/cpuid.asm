bits 32

section .text
    global cpuid_unsupported
    global cpuid

cpuid_unsupported:
    pushfd                               ; Save EFLAGS
    pushfd                               ; Store EFLAGS
    xor dword [esp], 0x00200000          ; Invert the ID bit in stored EFLAGS
    popfd                                ; Load stored EFLAGS (with ID bit inverted)
    pushfd                               ; Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ; eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax, [esp]                       ; eax = whichever bits were changed
    popfd                                ; Restore original EFLAGS
    and eax, 0x00200000                  ; eax = zero if ID bit can't be changed, else non-zero
    ret

cpuid:
    push ebx

    ; save code
    mov eax, dword [esp + 8]
    cpuid

    ; save result
    mov dword [.esp], esp
    mov esp, dword [esp + 12]
    lea esp, [esp + 4 * 4]
    push edx
    push ecx
    push ebx
    push eax
    mov esp, dword [.esp]

    pop ebx
    ret

section .bss
.esp: resd 1