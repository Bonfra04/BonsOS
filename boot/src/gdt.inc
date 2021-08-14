%ifndef __GDT_INC__
%define __GDT_INC__

GDT64.Selector.Kernel.Data  equ     0x10    ; 64-bit mode (kernel data)
GDT64.Selector.Kernel.Code  equ     0x08    ; 64-bit mode (kernel code)
GDT64.Selector.User.Data    equ     0x18    ; 64-bit mode (user data)
GDT64.Selector.User.Code    equ     0x20    ; 64-bit mode (user code)

struc GDT.Descriptor
    .LimitLow:          resw 1
    .BaseLow:           resw 1
    .BaseMiddle:        resb 1
    .Access:            resb 1
    .LimitHighFlags:    resb 1   ; LimitHigh (4 bits) + Flags (4 bits)
    .BaseHigh:          resb 1
endstruc

;-----------------------------------------------------------------------------
; Global Descriptor Table used in 64-bit mode
;-----------------------------------------------------------------------------
align 8
GDT64.Table:

; Null descriptor
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0x0000
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 0x00
    at GDT.Descriptor.LimitHighFlags,   db 0x00
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; kernel: code segment descriptor (selector = 0x08)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0x0000
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10011010b
    at GDT.Descriptor.LimitHighFlags,   db 00100000b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; kernel: data segment descriptor (selector = 0x10)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0x0000
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10010010b
    at GDT.Descriptor.LimitHighFlags,   db 00000000b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

GDT64.Table.Size equ ($ - GDT64.Table)

; GDT64 table pointer
GDT64.Table.Pointer:
    dw GDT64.Table.Size - 1    ; Limit = offset of last byte in table
    dq GDT64.Table             ; Address of table copy

%endif ; __GDT_INC__