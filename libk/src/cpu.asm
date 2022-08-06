bits 64

section .text
    global rdmsr
    global wrmsr
    global cpuid
    global get_flags

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

;-----------------------------------------------------------------------------
; @function     wrmsr
; @brief        Write to the model-specific register.
; @reg[in]      rdi     The MSR register id to write.
; @reg[in]      rsi     The value to write.
;-----------------------------------------------------------------------------
wrmsr:
    mov ecx, edi

    mov rax, rsi
    mov rdx, rax
    shr rdx, 32

    wrmsr

    ret

;-----------------------------------------------------------------------------
; @brief        Return the results of the CPUID instruction.
; @reg[in]      rdi     The cpuid group code.
; @reg[in]      rsi     pointer to a registers4_t struct.
;-----------------------------------------------------------------------------
cpuid:
    push rbx

    mov rax, rdi
    cpuid

    mov [rsi + 8 * 0],  rax
    mov [rsi + 8 * 1],  rbx
    mov [rsi + 8 * 2],  rcx
    mov [rsi + 8 * 3],  rdx

    pop rbx
    ret

get_flags:
    pushfq
    pop rax
    ret
