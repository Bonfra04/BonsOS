section .text
    global uart_init
    global uart_putc
    global uart_getc

uart_init:
    push rax
    push rdx

    mov dx, 0x3f9
    ; IER int off
    xor al, al
    out dx, al
    ; LCR set divisor mode
    mov al, 0x80
    add dl, 2
    out dx, al
    ; DLL divisor lo 115200
    mov al, 1
    sub dl, 3
    out dx, al
    ; DLH divisor hi
    xor al, al
    inc dl
    out dx, al
    ; FCR fifo off
    inc dl
    out dx, al
    ; LCR 8N1, break on
    mov al, 0x43
    inc dl
    out dx, al
    ; MCR Aux out 2
    mov al, 0x8
    inc dl
    out dx, al
    ; clear receiver/transmitter
    xor al, al
    sub dl, 4
    in al, dx

    pop rax
    pop rdx
    ret

; Param RDI[low8] => character to put
uart_putc:
    push rax
    push rcx
    push rdx
    mov ecx, 10000
    mov dx, 0x3fd
.one:
    in al, dx
    pause
    cmp al, 0xff
    je .two
    dec ecx
    jz .two
    and al, 0x20
    jz .one

    mov ax, di
    sub dl, 5
    out dx, al
.two:

    pop rdx
    pop rcx
    pop rax
    ret

uart_getc:
    push rdx

    xor rax, rax

    mov dx, 0x3fd
.one:
    pause
    in al, dx
    and al, 1
    jz .one
    sub dl, 5
    in al, dx

    cmp al, 13 ; '\r'
    jne .done
    mov al, 10 ; '\n'
.done:

    pop rdx
    ret