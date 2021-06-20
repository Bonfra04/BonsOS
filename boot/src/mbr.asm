org 0x7A00
bits 16

start:
    cli                         ; Disable interrupts
    xor ax, ax                  ; 0 AX
    mov ds, ax                  ; Set Data Segment to 0
    mov es, ax                  ; Set Extra Segment to 0
    mov fs, ax                  ; Set More Extra Segment to 0
    mov gs, ax                  ; Set Still More Extra Segment to 0
    mov ss, ax                  ; Set Stack Segment to 0
    mov esp, 0x7A00             ; Set Stack Pointer

.copyLower:
    mov cx, 0x0100              ; 256 WORDs in MBR
    mov si, 0x7C00              ; Current MBR Address
    mov di, 0x7A00              ; New MBR Address
    rep movsw                   ; Copy MBR

.jumpToRelocated:
    jmp 0 : lowStart            ; Jump to new Address

lowStart:
    sti                         ; Start interrupts

.saveBootDrive:
    call FixBootDrive           ; Some BIOSes don't pass the correct boot drive number
    mov byte [bootDrive], dl    ; Save BootDrive

.supportIntsCheck:
    call CheckInt13Support      ; Check if the BIOS supports int 13h extensions
    jc error.noInt13

.checkPartitions:               ; Check Partition Table For Bootable Partition
    mov bx, PT1                 ; Base = Partition Table Entry 1
    mov cx, 4                   ; There are 4 Partition Table Entries

.CKPTloop:
    mov al, byte [word bx]      ; Get Boot indicator bit flag
    test al, 0x80               ; Check For Active Bit
    jnz .CKPTFound              ; We Found an Active Partition
    add bx, 0x10                ; Partition Table Entry is 16 Bytes
    dec cx                      ; Decrement Counter
    jnz .CKPTloop               ; Loop
    jmp error.noBootablePartition

.CKPTFound:
    mov word [PToff], bx    ; Save Offset
    add bx, 0x08            ; Increment Base to LBA Address

.ReadVBR:
    mov dl, byte [bootDrive]    ; Bootdrive
    mov ax, 1                   ; Sectors to read
    mov ecx, dword [bx]         ; LBA address
    mov ebx, 0x7C00             ; Address to load to
    call ReadSectorsLBA         ; Read Sector
    jc error.diskReadFailed
    mov si, Message.Status.VBRLoaded
    call BiosPrint

.jumpToVBR:
    cmp word [0x7DFE], 0xAA55   ; Check Boot Signature
    jne error.noBootSignature   ; Error if not Boot Signature
    mov si, Message.Status.JumpingToVBR
    call BiosPrint
    mov si, word [PToff]        ; Set DS:SI to Partition Table Entry
    mov dl, byte [bootDrive]    ; Set DL to Drive Number
    jmp 0x7C00                  ; Jump To VBR

error:
    jmp hang

.noInt13:
    mov si, Message.Error.NoInt13
    call BiosPrint
    jmp hang

.diskReadFailed:
    mov si, Message.Error.DiskReadFailed
    call BiosPrint
    jmp hang

.noBootSignature:
    mov si, Message.Error.NoBootSignature
    call BiosPrint
    jmp hang

.noBootablePartition:
    mov si, Message.Error.NoBootablePartition
    call BiosPrint
    jmp hang

hang:
    cli
    hlt
    jmp hang

%include "include/disk.inc"

times 218 - ($-$$) db 0         ; Pad for disk time stamp
DiskTimeStamp times 8 db 0      ; Disk Time Stamp

%include "include/bios_print.inc"

;*************************************;
; Fix incorrect bootdrive value in dl ;
; Returns:                            ;
;   dl => Correct disk value          ;
;*************************************;
FixBootDrive:
    ; If the value the BIOS passed is <0x80, assume it is an incorrect value
    test dl, 0x80
    jz .fixDrive
    ; Drive numbers 0x80..0x8F should be valid
    test dl, 0x70
    jz .done
.fixDrive:
    mov dl, 0x80                ; Set correct value to dl
.done
    ret

;****************************************;
; Check if int13 extension are supported ;
; Returns:                               ;
;   cf => Set if not supported           ;
; Kills:                                 ;
;   ah, bx                               ;
;****************************************;
CheckInt13Support:
    clc                         ; Clear flag
    mov ah, 0x41
    mov bx, 0x55aa
    int 0x13
    jc .done
    cmp bx, 0xaa55
    je .done
    stc                         ; Set flag
.done
    ret

Message.Error.NoInt13 db "No int 13h extensions support.", 13, 10, 0
Message.Error.NoBootablePartition db "No bootable partition found.", 13, 10, 0
Message.Error.NoBootSignature db "No boot signature found.", 13, 10, 0
Message.Error.DiskReadFailed db "Disk read failed", 13, 10, 0
Message.Status.VBRLoaded db "VBR loaded!", 13, 10, 0
Message.Status.JumpingToVBR db "Jumping to VBR", 13, 10, 0

bootDrive db 0                  ; Drive Number Variable
PToff dw 0                      ; Partition Table Entry Offset

times 0x1B4 - ($-$$) db 0       ; Pad For MBR Partition Table

UID times 10 db 0               ; Unique Disk ID
PT1 times 16 db 0               ; First Partition Entry
PT2 times 16 db 0               ; Second Partition Entry
PT3 times 16 db 0               ; Third Partition Entry
PT4 times 16 db 0               ; Fourth Partition Entry
 
dw 0xAA55                       ; Boot Signature
