%ifndef __FAT12_EXT_INC__
%define __FAT12_EXT_INC__

%define BUFFER_OFFSET Mem.Loader.FAT16EXT.LoadBuffer
%define BUFFER_SEG LIN_TO_FAR_ADDR(BUFFER_OFFSET)

bits 16

;******************************************************;
; Copy a cluster from the buffer to a specific address ;
; Parameters:                                          ;
;   ebx => Address to copy cluster to                  ;
;******************************************************;
CopyCluster:
    pushad
.prepareProtected32Mode:

    ; Enable protected mode.
    mov eax, cr0
    or eax, (1 << 0)    ; CR.PE
    mov cr0, eax

    ; Do a far jump to switch to 32-bit protected mode.
    jmp GDT32.Selector.Code32 : .switchToProtected32Mode

bits 32
.switchToProtected32Mode:
    ; Initialize all data segment registers with the 32-bit protected mode
    ; data segment selector.
    mov ax, GDT32.Selector.Data32
    mov ds, ax
    mov es, ax
    mov ss, ax

.copyCluster:
    ; calculate sizeof cluster in byte into "cx"
    xor ecx, ecx
    xor ax, ax
    mov al, byte [bpb_SectorsPerCluster]
    mul word [bpb_BytesPerSector]
    mov cx, ax

    ; copy from esi, to edi
    mov esi, BUFFER_OFFSET
    mov edi, ebx ; ebx contains the address

    ; copy cx bytes from esi to edi
    rep movsb

.prepareProtected16Mode:
    ; Before we can switch back to real mode, we have to switch to
    ; 16-bit protected mode.
    jmp GDT32.Selector.Code16 : .switchToProtected16Mode

bits 16
.switchToProtected16Mode:
    ; Initialize all data segment registers with the 16-bit protected mode
    ; data segment selector.
    mov ax, GDT32.Selector.Data16
    mov ds, ax
    mov es, ax
    mov ss, ax

.prepareRealMode:
    ; Disable protected mode.
    mov eax, cr0
    and eax, ~(1 << 0)   ; CR0.PE
    mov cr0, eax

    ; Do a far jump to switch back to real mode.
    jmp 0 : .switchToRealMode

 .switchToRealMode:
    ; Restore real mode data segment registers.
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    popad
    ret

;************************************;
; Load file                          ;
; Parameters:                        ;
;   es:si => File name               ;
;   ebx => Buffer to load file to    ;
; Returns:                           ;
;   ax => -1 on error, 0 on success  ;
;************************************;
LoadFileExt:
.findFile:
    ; find file using name in es:si
    call FindFile
    cmp ax, -1
    je .done ; file not found

.loadFilePre:
    call PrepareFile

.nextCluster:
    push ebx
    mov ebx, BUFFER_SEG
    call LoadNextCluster
    pop ebx

    call CopyCluster

    call FileReadCompleated
    jc .success

    ; increase address
    xor eax, eax
    mov al, byte [bpb_SectorsPerCluster]
    mul word [bpb_BytesPerSector]
    add ebx, eax

    jmp .nextCluster

.success:
    xor ax, ax
.done
    ret

%endif ; __FAT12_EXT_INC__

