bits 64

section .text
    global cpuid
    global rdmsr
    global wrmsr
    global set_pagetable
    global invalidate_page
    global enable_interrupts
    global disable_interrupts
    global halt
    global invalid_opcode
    global fatal

;-----------------------------------------------------------------------------
; @function     cpuid
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

;-----------------------------------------------------------------------------
; @function     rdmsr
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
; @function     set_pagetable
; @brief        Update the CPU's page table register.
; @reg[in]      rdi     The physical address containing the new pagetable.
;-----------------------------------------------------------------------------
set_pagetable:
    mov cr3, rdi
    ret

;-----------------------------------------------------------------------------
; @function     invalidate_page
; @brief        Invalidate the page containing a virtual address.
; @reg[in]      rdi     The virtual address of the page to invalidate.
;-----------------------------------------------------------------------------
invalidate_page:
    invlpg [rdi]
    ret

;-----------------------------------------------------------------------------
; @function     enable_interrupts
; @brief        Enable interrupts.
;-----------------------------------------------------------------------------
enable_interrupts:
    sti
    ret

;-----------------------------------------------------------------------------
; @function     disable_interrupts
; @brief        Disable interrupts.
;-----------------------------------------------------------------------------
disable_interrupts:
    cli
    ret

;-----------------------------------------------------------------------------
; @function     halt
; @brief        Halt the CPU until an interrupt occurs.
;-----------------------------------------------------------------------------
halt:
    hlt
    ret

;-----------------------------------------------------------------------------
; @function     invalid_opcode
; @brief        Raise an invalid opcode exception.
;-----------------------------------------------------------------------------
invalid_opcode:
    int 6
    ret

;-----------------------------------------------------------------------------
; @function     fatal
; @brief        Raise a fatal interrupt that hangs the system.
;-----------------------------------------------------------------------------
fatal:
    int 0xff
    ret
