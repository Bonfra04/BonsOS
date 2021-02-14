%ifndef __PAGING_INC__
%define __PAGING_INC__

bits 16

;=============================================================================
; SetupPageTables
;
; Set up a page table for 64-bit mode.
;
; This procedure creates an identity-mapped page table for the first 10MiB of
; physical memory.
;
; Killed registers:
;   None
;=============================================================================
SetupPageTables:
    ; Constants for page table bits
    .Present        equ 1 << 0
    .ReadWrite      equ 1 << 1
    .WriteThru      equ 1 << 3
    .CacheDisable   equ 1 << 4
    .AttribTable    equ 1 << 7      ; valid only on PT entries
    .LargePage      equ 1 << 7      ; valid only on PDT entries
    .Guard          equ 1 << 9      ; use a bit ignored by the CPU
    .StdBits        equ .Present | .ReadWrite

    pusha

    ; Set segment to the root of the page table.
    mov ax, Mem.PageTable >> 4
    mov es, ax

    .clearMemory:
        ; Clear all memory used to hold the page tables.
        cld
        xor eax, eax
        xor edi, edi
        mov ecx, (Mem.PageTable.End - Mem.PageTable) >> 2
        rep stosd

    .makeTables:
        ; PML4T entry 0 points to the PDPT.
        mov di, Mem.PageTable.PML4T & 0xffff
        mov dword [es:di], Mem.PageTable.PDPT | .StdBits

        ; PDPT entry 0 points to the PDT.
        mov di, Mem.PageTable.PDPT & 0xffff
        mov dword [es:di], Mem.PageTable.PDT | .StdBits

        ; PDT entry 0 maps the first 2MiB using 4KiB pages.
        mov di, Mem.PageTable.PDT & 0xffff
        mov dword [es:di + 0x00], Mem.PageTable.PT | .StdBits

        ; PDT entries 1 through 5 map the next 8MiB using 2MiB pages.
        ; This memory holds the kernel image and its stack.
        mov dword [es:di + 0x08], 0x00200000 | .StdBits | .LargePage
        mov dword [es:di + 0x10], 0x00400000 | .StdBits | .LargePage
        mov dword [es:di + 0x18], 0x00600000 | .StdBits | .LargePage
        mov dword [es:di + 0x20], 0x00800000 | .StdBits | .LargePage
        mov dword [es:di + 0x28], 0x00A00000 | .StdBits | .LargePage
        mov dword [es:di + 0x30], 0x00C00000 | .StdBits | .LargePage
        mov dword [es:di + 0x38], 0x00E00000 | .StdBits | .LargePage

        ; Prepare to create 4K-page table entries for the first 2MiB.
        mov di, Mem.PageTable.PT & 0xffff
        mov eax, .StdBits
        mov cx, 512     ; 512 entries in first page covering 2MiB

    .makePage:
        ; Loop through each page table entry, incrementing the physical
        ; address by one page each time.
        mov [es:di], eax       ; store physical address + .StdBits
        add eax, 0x1000    ; next physical address
        add di, 8         ; next page table entry
        loop .makePage

    .initPageRegister:
        ; CR3 is the page directory base register.
        mov edi, Mem.PageTable
        mov cr3, edi

    .done:
        ; Clear the upper bits of 32-bit registers we used.
        xor eax, eax
        xor ecx, ecx
        xor edi, edi

        popa
        ret

%endif ; __PAGING_INC__