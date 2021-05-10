%ifndef __VESA_INC__
%define __VESA_INC__

bits 16

;**********************;
; Enable a VESA mode   ;
; Parameters:          ;
;   bx => mode number  ;
; Returns:             ;
;   cf => Set if error ;
;**********************;
EnableVesaMode:
    push es
    mov ax, 0x4F02
    or bx, 0x4000   ; enable LFB
    mov di, 0       ; not sure if some BIOSes need this
    int 0x10
    pop es
 
    cmp ax, 0x4F    ; check if success
    jne .error

.done:
    clc
    ret
.error:
    stc
    ret

;**********************;
; Find a VESA mode     ;
; Parameters:          ;
;   ax => width        ;
;   bx => height       ;
;   cl => bpp          ;
; Returns:             ;
;   cf => Set if error ;
;   dx => Mode number  ;
;   ecx => FrameBuffer ;
;   ax => pitch        ;
;**********************;
DetectVesaMode:
    mov [.width], ax
    mov [.height], bx
    mov [.bpp], cl

    mov al, 'V'
    mov [Mem.VESA.Info], al
    mov al, 'B'
    mov [Mem.VESA.Info + 1], al
    mov al, 'E'
    mov [Mem.VESA.Info + 2], al
    mov al, '2'
    mov [Mem.VESA.Info + 3], al

    push es             ; preserve es
    mov ax, 0x4F00      ; get VBE BIOS info (es:di address)
    mov di, Mem.VESA.Info
    int 0x10
    pop es              ; restore ES

    cmp ax, 0x004F      ; BIOS doesn't support VBE?
    jne .error

    mov ax, word[Mem.VESA.Info + 14]
    mov [.offset], ax
    mov ax, word[Mem.VESA.Info + 14 + 2]
    mov [.segment], ax

    mov ax, [.segment]
    mov fs, ax
    mov si, [.offset]

.find_mode:
    mov dx, [fs:si]     ; retrive data from fs:si
    add si, 2           ; increase si to point to the next value
    mov [.offset], si   ; save si value
    mov [.mode], dx     ; save retrieved data
    mov ax, 0           ; reset fs to 0
    mov fs, ax

    cmp word [.mode], 0xFFFF
    je .error           ; no suitable mode found

    push es             ; preserve es
    mov ax, 0x4F01      ; get VBE mode info
    mov cx, [.mode]
    mov di, Mem.VESA.ModeInfo
    int 0x10             
    pop es              ; restore ES

    cmp ax, 0x4F
    jne .error          ; error retrieving mode info 

    ; check bpp
    mov al, [Mem.VESA.ModeInfo + 25]
    cmp al, [.bpp]
    jne .next_mode

    ; check width
    mov ax, [Mem.VESA.ModeInfo + 18]
    cmp ax, [.width]
    jne .next_mode

    ; check height
    mov ax, [Mem.VESA.ModeInfo + 20]
    cmp ax, [.height]
    jne .next_mode

    mov ecx, [Mem.VESA.ModeInfo + 40]   ; framebuffer
    mov ax, [Mem.VESA.ModeInfo + 16]    ; pitch
    mov dx, [.mode]
    jmp .done

.next_mode:
    mov ax, [.segment]  ; prepare segment for the next query
    mov fs, ax
    mov si, [.offset]   ; prepare offset for the next query
    jmp .find_mode

.done:
    clc
    ret
.error:
    stc
    ret

.width      dw 0
.height     dw 0
.bpp        db 0
.segment    dw 0
.offset     dw 0
.mode       dw 0

%endif ; __VESA_INC__