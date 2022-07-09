bits 64

section .text
    global rdmsr

;-----------------------------------------------------------------------------
; @brief        Read the model-specific register and return the result.
; @reg[in]      rdi     The MSR register id to read.
; @reg[out]     rax     The contents of the requested MSR.
;-----------------------------------------------------------------------------
rdmsr:
    mov rcx, rdi

    rdmsr

    shl rdx, 32
    or rax, rdx
    ret
