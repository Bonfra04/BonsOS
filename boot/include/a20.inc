%ifndef __A20_INC__
%define __A20_INC__

bits 16

;*****************************;
; Enable the A20 address line ;
; Returns:                    ;
;   cf => Set if enable       ;
;*****************************;
EnableA20:
    pusha

    ; Check if the A20 line is already enabled.
    call TestA20
    jc .done

    .attempt1:
        ; Attempt enabling with the BIOS.
        mov ax, 0x2401
        int 0x15

        ; Check if A20 line is now enabled.
        call TestA20
        jc .done

    .attempt2:
        ; Attempt enabling with the keyboard controller.
        call .attempt2.wait1

        ; Disable keyboard
        mov al, 0xad
        out 0x64, al
        call .attempt2.wait1

        ; Read from input
        mov al, 0xd0
        out 0x64, al
        call .attempt2.wait2

        ; Get keyboard data
        in al, 0x60
        push eax
        call .attempt2.wait1

        ; Write to output
        mov al, 0xd1
        out 0x64, al
        call .attempt2.wait1

        ; Send data
        pop eax
        or al, 2
        out 0x60, al
        call .attempt2.wait1

        ; Enable keyboard
        mov al, 0xae
        out 0x64, al
        call .attempt2.wait1

        ; Check if the A20 line is now enabled.
        call TestA20
        jc .done

        jmp .attempt3

    .attempt2.wait1:
        in al, 0x64
        test al, 2
        jnz .attempt2.wait1
        ret

    .attempt2.wait2:
        in al, 0x64
        test al, 1
        jz .attempt2.wait2
        ret

    .attempt3:
        ; Attempt enabling with the FAST A20 feature.
        in al, 0x92
        or al, 2
        out 0x92, al
        xor ax, ax

        ; Check if A20 line is now enabled.
        call TestA20 

    .done:
        popa
        ret

;******************************************;
; Check if the A20 address line is enabled ;
; Returns:                                 ;
;   cf => Set if enable                    ;
;******************************************;
TestA20:
    push    ds
    push    es
    pusha

    ; Initialize return result to "not enabled".
    clc

    ; Set es segment register to 0x0000.
    xor ax, ax
    mov es, ax

    ; Set ds segment register to 0xffff.
    not ax
    mov ds, ax

    ; If the A20 line is disabled, then es:di and ds:si will point to the same
    ; physical memory location due to wrap-around at 1 MiB.
    ;
    ; es:di = 0000:0500 = 0x0000 * 16 + 0x0500 = 0x00500 = 0x0500
    ; ds:si = ffff:0510 = 0xffff * 16 + 0x0510 = 0x10500 = 0x0500
    mov di, 0x0500
    mov si, 0x0510

    ; Preserve the original values stored at es:di and ds:si.
    mov ax, [es:di]
    push ax
    mov ax, [ds:si]
    push ax

    ; Write different values to each logical address.
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xff

    ; If a store to ds:si changes the value at es:di, then memory wrapped and
    ; A20 is not enabled.
    cmp byte [es:di], 0xff

    ; Restore the original values stored at es:di and ds:si.
    pop ax
    mov [ds:si], ax
    pop ax
    mov [es:di], ax

    je .done

    .enabled:
        ; Set the carry flag to indicate the A20 line is enabled.
        stc

    .done:
        popa
        pop     es
        pop     ds

        ret

%endif ; __A20_INC__