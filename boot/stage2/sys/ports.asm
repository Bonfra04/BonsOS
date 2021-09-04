bits 64

section .text
    global inportb
    global outportb

;-----------------------------------------------------------------------------
; @function     inportb
; @brief        Retrieve a byte value from an input port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[out]     rax     Byte value read from the port.
;-----------------------------------------------------------------------------
inportb:
    mov dx, di
    xor rax, rax
    in al, dx
    ret

;-----------------------------------------------------------------------------
; @function     outportb
; @brief        Write a byte value to an output port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[in]      rsi     Byte value to write to the port.
;-----------------------------------------------------------------------------
outportb:
    mov dx, di
    mov ax, si
    out dx, al
    ret
