%ifndef __GDT_INC__
%define __GDT_INC__

GDT32.Selector.Code32       equ     0x08    ; 32-bit protected mode (code)
GDT32.Selector.Data32       equ     0x10    ; 32-bit protected mode (data)
GDT32.Selector.Code16       equ     0x18    ; 16-bit protected mode (code)
GDT32.Selector.Data16       equ     0x20    ; 16-bit protected mode (data)

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
; Global Descriptor Table used in 32-bit protected mode
;-----------------------------------------------------------------------------
align 4
GDT32.Table:

; Null descriptor
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0x0000
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 0x00
    at GDT.Descriptor.LimitHighFlags,   db 0x00
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; 32-bit protected mode - code segment descriptor (selector = 0x08)
; (Base=0, Limit=4GiB-1, RW=1, DC=0, EX=1, PR=1, Priv=0, SZ=1, GR=1)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0xffff
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10011010b
    at GDT.Descriptor.LimitHighFlags,   db 11001111b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; 32-bit protected mode - data segment descriptor (selector = 0x10)
; (Base=0, Limit=4GiB-1, RW=1, DC=0, EX=0, PR=1, Priv=0, SZ=1, GR=1)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0xffff
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10010010b
    at GDT.Descriptor.LimitHighFlags,   db 11001111b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; 16-bit protected mode - code segment descriptor (selector = 0x18)
; (Base=0, Limit=1MiB-1, RW=1, DC=0, EX=1, PR=1, Priv=0, SZ=0, GR=0)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0xffff
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10011010b
    at GDT.Descriptor.LimitHighFlags,   db 00000001b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

; 16-bit protected mode - data segment descriptor (selector = 0x20)
; (Base=0, Limit=1MiB-1, RW=1, DC=0, EX=0, PR=1, Priv=0, SZ=0, GR=0)
istruc GDT.Descriptor
    at GDT.Descriptor.LimitLow,         dw 0xffff
    at GDT.Descriptor.BaseLow,          dw 0x0000
    at GDT.Descriptor.BaseMiddle,       db 0x00
    at GDT.Descriptor.Access,           db 10010010b
    at GDT.Descriptor.LimitHighFlags,   db 00000001b
    at GDT.Descriptor.BaseHigh,         db 0x00
iend

GDT32.Table.Size equ ($ - GDT32.Table)

GDT32.Table.Pointer:
    dw GDT32.Table.Size - 1    ; Limit = offset of last byte in table
    dd GDT32.Table

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
; This pointer references Mem.GDT, not GDT64.Table, because that's where the
; 64-bit mode GDT will reside after copying it.
GDT64.Table.Pointer:
    dw GDT64.Table.Size - 1    ; Limit = offset of last byte in table
    dq Mem.GDT                 ; Address of table copy

%endif ; __GDT_INC__