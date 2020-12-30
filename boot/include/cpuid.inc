%ifndef __CPUID_INC__
%define __CPUID_INC__

bits 16

;**************************************************;
; Detect if the cpu supports the CPUID instruction ;
; Returns:                                         ;
;   cf => Set if supported                         ;
;**************************************************;
HasCPUID:
    pusha

    ; Copy flags to eax and ecx.
    pushfd
    pop eax
    mov ecx, eax

    ; Set flag 21 (the ID bit)
    xor eax, (1 << 21)
    push eax
    popfd

    ; Copy flags back to eax. If CPUID is supported, bit 21 will still be set.
    pushfd
    pop eax

    ; Restore the original flags from ecx.
    push ecx
    popfd

    ; Initialize the return flag (carry) to unsupported.
    clc

    ; If eax and ecx are equal, then flag 21 didn't remain set, and CPUID is
    ; not supported.
    xor eax, ecx
    jz .done       ; CPUID is not supported

.supported:
    stc

.done:
    popa
    ret

;******************************************;
; Detect if the cpu supports the Long Mode ;
; Returns:                                 ;
;   cf => Set if supported                 ;
;******************************************;
DetectLongMode:
    pusha

    ; Initialize the return flag (carry) to unsupported.
    clc

    ; Is the processor info function supported?
    mov eax, 0x80000000  ; Get Highest Extended Function Supported
    cpuid
    cmp eax, 0x80000001
    jb .done

    ; Use processor info function to see if 64-bit mode is supported.
    mov eax, 0x80000001  ; Extended Processor Info and Feature Bits
    cpuid
    test edx, 1 << 29   ; 64-bit mode bit
    jz .done

.supported:
    stc

.done:
    popa
    ret

%endif ; __CPUID_INC__