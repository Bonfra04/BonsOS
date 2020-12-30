bits 64

section .text
    global inportb
    global outportb
    global inportw
    global outportw
    global inportd
    global outportd


;-----------------------------------------------------------------------------
; @function     io_inb
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
; @function     inportb
; @brief        Write a byte value to an output port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[in]      rsi     Byte value to write to the port.
;-----------------------------------------------------------------------------
outportb:
    mov dx, di
    mov ax, si
    out dx, al
    ret

;-----------------------------------------------------------------------------
; @function     io_inw
; @brief        Retrieve a 16-bit word value from an input port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[out]     rax     Word value read from the port.
;-----------------------------------------------------------------------------
inportw:
    mov dx, di
    xor rax, rax
    in ax, dx
    ret

;-----------------------------------------------------------------------------
; @function     io_outw
; @brief        Write a 16-bit word value to an output port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[in]      rsi     Word value to write to the port.
;-----------------------------------------------------------------------------
outportw:
    mov dx, di
    mov ax, si
    out dx, ax
    ret

;-----------------------------------------------------------------------------
; @function     io_ind
; @brief        Retrieve a 32-bit dword value from an input port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[out]     rax     Dword value read from the port.
;-----------------------------------------------------------------------------
inportd:
    mov dx, di
    xor rax, rax
    in eax, dx
    ret

;-----------------------------------------------------------------------------
; @function     io_outd
; @brief        Write a 32-bit dword value to an output port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[in]      rsi     Dord value to write to the port.
;-----------------------------------------------------------------------------
outportd:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret