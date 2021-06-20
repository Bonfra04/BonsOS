org 0x7C00
bits 16

%include "include/memory_layout.inc"
%include "include/fat16_bpb.inc"    ; FAT16 bios parameter block containing all disk informations

boot:
.init:
    mov word [partition_offset], si ; Save partition table offset
    mov byte [bpb_DriveNumber], dl  ; Save drive number

.loadSecondStage:
    call LoadRoot                   ; Loads in memory FAT16 root directory
    mov ebx, dword LIN_TO_FAR_ADDR(Mem.Loader2)
    mov si, ImageName
    call LoadFile                   ; Loads at address ebx the file with name si

.jumpToSecondStage:
    mov si, word [partition_offset]
    mov dl, byte [bpb_DriveNumber]
    jmp 0 : Mem.Loader2             ; Far jump to the loaded file

partition_offset dw 0x0000
ImageName db "LOADER  BIN"          ; Filename of the second stage bootloader [8 chars name] + [3 chars extension] padded with space

%include "include/fat16.inc"        ; Utilities to interact with FAT16 

times 510-($-$$) db 0               ; Pad with zeros to a full secotr (512 byte)
dw 0xAA55                           ; Boot signature