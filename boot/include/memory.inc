%ifndef __MEMORY_INC__
%define __MEMORY_INC__

struc MemoryMapEntry
    .baseAddress    resq 1 ; base address of address range
    .length         resq 1 ; length of address range in bytes
    .type           resd 1 ; type of address range
    .acpi_null      resd 1 ; reserved
endstruc

;*******************************************;
; Get memory map from bios                  ;
; Parameters:                               ;
;   es:di => memory map destination address ;
; Returns:                                  ;
;   bp => entry count                       ;
;   cf => Set if error                      ;
;*******************************************;
BiosGetMemoryMap:
    push eax
    push ebx
    push ecx
    push edx

    ; clear registers
    xor ebx, ebx
    xor bp, bp

    ; call interrupt
    mov edx, 'PAMS'
    mov ecx, 24     ; memory map entry struct is 24 bytes
    mov eax, 0xE820
    int 0x15        ; get first entry
    ; error handling
    jc.error
    cmp eax, 'PAMS' ; bios returns SMAP in eax
    jne .error
    test ebx, ebx   ; if ebx=0 then list is one entry long; bail out
    je .error
    jmp .start

.next_entry:
    mov edx, 'PAMS'
    mov ecx, 24     ; memory map entry struct is 24 bytes
    mov eax, 0xE820
    int 0x15
.start:
    jcxz .skip_entry    ; if actual returned bytes is 0, skip entry
.notext:
    mov ecx, [es:di + MemoryMapEntry.length]    ; get length (low dword)
    test ecx, ecx                               ; if length is 0 skip it
    jne short .good_entry
    mov ecx, [es:di + MemoryMapEntry.length + 4]; get length (upper dword)
    jecxz .skip_entry                           ; if length is 0 skip it
.good_entry:
    inc bp          ; increment entry count
    add di, 24      ; point di to next entry in buffer
.skip_entry:
    cmp ebx, 0      ; if ebx return is 0, list is done
    jne .next_entry ; get next entry
    jmp .done

.error:
    stc
.done:
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

;*****************************************;
; Get memory size from bios               ;
; Returns:                                ;
;   ax => KB between 1MB and 16MB         ;
;   bx => number of 64K blocks above 16MB ;
;   ax => -1 on error                     ;
;   bx => 0 on error                      ;
;*****************************************;
BiosGetMemorySize:
    ; save registers
    push ecx
    push edx

    ; clear registers
    xor ecx, ecx
    xor edx, edx

    ; call bios function
    mov ax, 0xE801
    int 0x15
    jc .error

    ; error handling
    cmp ah, 0x86        ; unsupported function
    je .error
    cmp ah, 0x80        ; invalid command
    je .error

    jcxz .use_ax        ; bios may have stored it in ax,bx or cx,dx. test if cx is 0
    mov ax, cx          ; its not, so it should contain mem size; store it
    mov bx, dx

.use_ax: ; mem size is in ax and bx already, return it
    ; restore registers
    pop edx
    pop ecx
    ret

.error:
    ; error codes
    mov ax, -1
    mov bx, 0
    ; restore registers
    pop edx
    pop ecx
    ret

%endif ; __MEMORY_INC__