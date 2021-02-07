bits 64

section .text
    global inportb
    global outportb
    global inportw
    global outportw
    global inportd
    global outportd
    global port_wait

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

;-----------------------------------------------------------------------------
; @function     inportw
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
; @function     outportw
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
; @function     inportd
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
; @function     outportd
; @brief        Write a 32-bit dword value to an output port.
; @reg[in]      rdi     Port number (0-65535).
; @reg[in]      rsi     Dord value to write to the port.
;-----------------------------------------------------------------------------
outportd:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

;-----------------------------------------------------------------------------
; @function     port_wait
; @brief        Write to an unused port to waste 1 io cycle
;-----------------------------------------------------------------------------
port_wait:
    mov al, 0
    out 0x80, al
    ret