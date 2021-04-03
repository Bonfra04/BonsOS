%ifndef __FAT12_INC__
%define __FAT12_INC__

; this file relies on a word variable called partition_offset
; partition_offset should contain the address in memory of
; the current partition entry in the table

bits 16

%define ROOT_OFFSET Mem.Loader.FAT16.Data
%define ROOT_SEG LIN_TO_FAR_ADDR(ROOT_OFFSET)

%define FAT_OFFSET Mem.Loader.FAT16.Data
%define FAT_SEG LIN_TO_FAR_ADDR(FAT_OFFSET)

%include "include/disk.inc"

;*********************************;
; Sets Rot Directory informations ;
; Returns:                        ;
;   ecx => location in sectors.   ;
;   eax => size in sectors        ;
;*********************************;
GetRootInfo:
    ; clear registers
    xor eax, eax
    xor ecx, ecx

    ; compute location of root directory in sectors and store in "ecx" 
    mov al, byte [bpb_NumberOfFATs]     ; number of FATs
    mul word [bpb_SectorsPerFAT]        ; sectors used by FATs
    add ax, word [bpb_ReservedSectors]  ; reserved sectors
    mov cx, ax                          ; move into cx
    mov ax, word [partition_offset]
    add cx, word [eax + 8]              ; offset from partition information

    ; compute size of root directory in sectors and store in "ax" 
    mov ax, 32                          ; 32 byte directory entry
    mul word [bpb_RootEntries]          ; total size of directory
    div word [bpb_BytesPerSector]       ; sectors used by directory

    ret

;***************************************;
; Load Root Directory Table to ROOT_SEG ;
;***************************************;
LoadRoot:
    call GetRootInfo

    ; read Root into memory at ROOT_SEG
    mov dl, byte [bpb_DriveNumber]
    mov ebx, dword ROOT_SEG
    call ReadSectorsLBA
    ret

;****************************;
; Loads FAT table to FAT_SEG ;
;****************************;
LoadFAT:
    pushad

    ; clear registers
    xor ax, ax
    xor cx, cx

    ; compute size of FAT and store in "ax" 
    mov ax, word [bpb_SectorsPerFAT]    ; number of FATs

    ; compute location of FAT and store in "ecx"
    mov cx, word [partition_offset]
    mov cx, word [ecx + 8]              ; offset from partition information
    add cx, word [bpb_ReservedSectors]

    ; read FAT into memory at FAT_SEG
    mov dl, byte [bpb_DriveNumber]
    mov ebx, dword FAT_SEG
    call ReadSectorsLBA

    popad
    ret

;***********************************************;
; Search for filename in root table             ;
; Parameters:                                   ;
;   si => File name                             ;
; Returns:                                      ;
;   ax => File index number in directory table. ;
;   di => Location of the file root entry       ;
;***********************************************;
FindFile:
    ; clear registers
    xor edi, edi

    ; store registers   
    push cx
    push bx
    push es

    mov bx, si                      ; save filename for later

    push ROOT_SEG >> 16             ; locate first root entry
    pop es
    mov di, ROOT_SEG                ; ES:DI is memory location of root dir
    mov cx, word [bpb_RootEntries]  ; load loop counter
    ; browse root directory for binary image
    cld                             ; clear direction flag
.loop:
    push cx
    mov cx, 11                      ; eleven character name. Image name is in SI
    mov si, bx                      ; image name is in BX
    push di
    rep cmpsb                       ; test for entry match [ DS:SI - ES:DI ]
    pop di
    je .Found
    pop cx
    add di, 32                      ; queue next directory entry
    loop .loop

.NotFound:
    ; set error code
    mov ax, -1
    ; restore registers and return
    pop es
    pop bx
    pop cx
    ret

.Found:
    ; return value into AX contains entry of file
    pop ax
    ; restore registers and return
    pop es
    pop bx
    pop cx
    ret

;**********************************************;
; Gain information about the file to be loaded ;
; Parameters:                                  ;
;   edi => Location of the file root entry     ;
;**********************************************;
PrepareFile:
    ; get starting cluster
    push word ROOT_SEG >> 16
    pop es
    mov ax, word [es:edi + 0x1A]   ; retrive cluster from root entry
    mov word [cluster], ax

    ; get 0th cluster address
    call GetRootInfo
    add eax, ecx
    mov dword [first_cluster_sector], eax

    call LoadFAT
    ret

;*********************************************;
; Read the current selected cluster in memory ;
; Parameters:                                 ;
;   ebx => Buffer to load file to             ;
;*********************************************;
LoadNextCluster:
    ; zero out registers for calculations
    xor cx, cx
    xor dx, dx

    ; convert the cluster in lba
    mov ax, word [cluster]
    sub ax, 2
    mov dl, byte [bpb_SectorsPerCluster]
    mul dx
    xchg cx, ax
    add ecx, dword [first_cluster_sector]

    ; sets the others parameters and read the disk
    mov dl, byte [bpb_DriveNumber]
    mov al, byte [bpb_SectorsPerCluster]
    call ReadSectorsLBA

    ; get next cluster from fat table
    xor eax, eax
    mov ax, word [cluster]
    mov dx, 2
    mul dx ; since fat table is an array of words (2 byte)

    push word FAT_SEG >> 16
    pop es
    mov dx, word [dword es:eax]
    mov word [cluster], dx

    ret

;*******************************************************;
; Check if the current reading file has been compleated ;
; Returns:                                              ;
;   cf => set on compleated                             ;
;*******************************************************;
FileReadCompleated:
    ; test if it was the last cluster
    cmp word [cluster], 0xFFFF
    je .complete
    cmp word [cluster], 0xFFF8
    je .complete

.incomplete:
    clc ; clear carry flag
    ret
.complete:
    stc ; set carry flag
    ret

;************************************;
; Load file                          ;
; Parameters:                        ;
;   es:si => File name               ;
;   ebx => Buffer to load file to    ;
; Returns:                           ;
;   ax => -1 on error, 0 on success  ;
;************************************;
LoadFile:
.findFile:
    ; find file using name in es:si
    call FindFile
    cmp ax, -1
    je .done ; file not found

.loadFilePre:
    call PrepareFile

.nextCluster:
    call LoadNextCluster

    call FileReadCompleated
    jc .success

    ; increase address
    ;xor eax, eax
    mov ax, word [bpb_SectorsPerCluster]
    mul word [bpb_BytesPerSector]
    mov ecx, eax
    shr eax, 4
    shl eax, 16
    and ecx, 0xF
    or eax, ecx
    add ebx, eax
    ; check if riporto
    mov eax, ebx
    and eax, (1 << 4)
    cmp eax, 0
    je .nextCluster
    ; fix riporto
    xor ebx, 1 << 4
    add ebx, 1 << 16
    jmp .nextCluster

.success:
    xor ax, ax
.done
    ret

cluster dw 0x0000
first_cluster_sector dd 0x00000000


%endif ; __FAT12_INC__