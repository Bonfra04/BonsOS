bits 64

section .text
    global memsetw

;-----------------------------------------------------------------------------
; @function     memsetw
; @brief        Fill a region of memory with a single 16-bit word value.
; @reg[in]      rdi     Address of the destination memory area.
; @reg[in]      rsi     Value of the word used to fill memory.
; @reg[in]      rdx     Number of words to set.
; @reg[out]     rax     Destination address.
; @killedregs   r8, rcx
;-----------------------------------------------------------------------------
memsetw:
    mov r8, rdi     ; Preserve the original destination address.
    mov rax, rsi    ; The value to store is the second parameter (rsi).
    mov rcx, rdx    ; Do a byte-by-byte store.
    rep stosw
    mov rax, r8     ; Return the original destination address.
    ret

