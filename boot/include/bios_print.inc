%ifndef __BIOS_PRINT_INC__
%define __BIOS_PRINT_INC__

bits 16

;************************;
; Parameters:            ;
;   si => string pointer ;
;************************;
BiosPrint:
    pusha
    .loop:
        lodsb
        or al, al
        jz .done
        mov ah, 0x0E
        int 0x10
        jmp .loop
    .done:
    popa
    ret

%endif ; __BIOS_PRINT_INC__