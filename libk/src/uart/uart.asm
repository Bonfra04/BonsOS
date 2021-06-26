section .text
    global uart_init

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
