%ifndef __DISK_INC__
%define __DISK_INC__

bits 16

;***********************************;
; Reads a series of sectors         ;
; Parameters:                       ;
;   dl => bootdrive                 ;
;   ax => sectors count             ;
;   ebx => address to load to       ;
;   ecx => LBA address              ;
; Returns:                          ;
;   cf => set if error              ;
;***********************************;
ReadSectorsLBA:
    push ebx
    mov word [LBA_Packet.block_cout], ax
    mov dword [LBA_Packet.transfer_buffer], ebx
    mov dword [LBA_Packet.lba_value], ecx
    mov si, LBA_Packet
    mov ah, 0x42    ; Read sectors function
    int 0x13
    pop ebx
    ret

align 4
LBA_Packet:
    .packet_size     db 0x10 ; use_transfer_64 ? 10h : 18h
    .reserved        db 0x00 ; always zero
    .block_cout      dw 0x00 ; number of sectors to read
    .transfer_buffer dd 0x00 ; address to load in ram
    .lba_value       dq 0x00 ; LBA addres value   

%endif ;__DISK_INC__