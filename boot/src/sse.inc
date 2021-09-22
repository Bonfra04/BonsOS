%ifndef __SSE_INC__
%define __SSE_INC__

bits 16

CheckSupportFX:
    pusha

    ; Initialize return result to error.
    clc

    ; Load CPU feature flags into ecx and edx.
    mov eax, 1
    cpuid

    ; Check for FXSAVE/FXRSTOR support.
    test edx, (1 << 24)
    jz .done

.success:
    ; Set the carry flag to enabled.
    stc
.done:
    popa
    ret

CheckSupportSSE:
    pusha

    ; Initialize return result to error.
    clc

    ; Load CPU feature flags into ecx and edx.
    mov eax, 1
    cpuid

    ; Check for SSE1 support.
    test edx, (1 << 25)
    jz .done

    ; Check for SSE2 support.
    test edx, (1 << 26)
    jz .done

.success:
    ; Set the carry flag to enabled.
    stc
.done:
    popa
    ret

ActivateSSE:
    pusha

    ; Enable hardware FPU with monitoring.
    mov eax, cr0
    and eax, ~(1 << 2)   ; turn off CR0.EM bit (x87 FPU is present)
    or eax, (1 << 1)     ; turn on CR0.MP bit (monitor FPU)
    mov cr0, eax

    ; Make sure FXSAVE/FXRSTOR instructions save the contents of the FPU
    ; MMX, XMM and MXCSR registers. Enable the use of SSE instructions.
    ; And indicate that the kernel is capable of handling SIMD floating-
    ; point exceptions.
    mov eax, cr4
    or eax, (1 << 9) | (1 << 10)    ; CR4.OFXSR, CR4.OSXMMEXCPT
    mov cr4, eax

    popa
    ret

%endif ; __SSE_INC__