;org 0x7e00
section .text
    global load
bits 16

%macro BiosPrintMacro 1
    mov si, word %1
    call BiosPrint
%endmacro

load:
.init:
    mov word [partition_offset], si ; Save partition table offset
    mov byte [boot_info+bootinfo.bootDevice], dl ; Save drive number

    mov dword [boot_info+bootinfo.memoryMapAddress], Mem.MemoryMap ; Save memory map address

    cli  ; Disable interrupts

    ; Clear all general purpose registers.
    xor ax, ax
    xor bx, bx
    xor cx, cx
    xor dx, dx
    xor si, si
    xor di, di
    xor bp, bp

    ; Initialize all segment registers to zero.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; create stack
    mov ss, ax
    mov sp, Mem.Loader.Stack.Top

    BiosPrintMacro String.Loading
    
.enableA20:
    call EnableA20
    jnc error.a20line

    BiosPrintMacro String.Status.A20Enabled

.detect64BitMode:
    call HasCPUID
    jnc error.noCPUID

    call DetectLongMode
    jnc error.no64BitMode

    BiosPrintMacro String.Status.CPU64Detected

.enableSSE:
    call CheckSupportFX
    jnc error.noFXinst

    call CheckSupportSSE
    jnc error.noSSE

    call ActivateSSE

    BiosPrintMacro String.Status.SSEEnabled

.loadGDT32:
    lgdt [GDT32.Table.Pointer]

.detectMemory:    
    call BiosGetMemorySize
    cmp ax, -1
    je error.undetectedMemory
    mov word [boot_info+bootinfo.memorySizeHigh], bx
    mov word [boot_info+bootinfo.memorySizeLow], ax

    mov di, Mem.MemoryMap
    call BiosGetMemoryMap
    jc error.undetectedMemory
    mov word [boot_info+bootinfo.memoryMapEntries], bp

    BiosPrintMacro String.Status.DetectMemory

.loadKernel:
    call LoadKernel
    jc error.kernelLoadFailed

    BiosPrintMacro String.Status.KernelLoaded

.loadGDT64:
    ; Copy the GDT to its memory layout location.
    mov si, GDT64.Table
    mov di, Mem.GDT
    mov cx, GDT64.Table.Size
    shr cx, 1
    rep movsw
    lgdt [GDT64.Table.Pointer]

.setupPageTables:
    call SetupPageTables
    ; Enable PAE paging.
    mov eax, cr4
    or eax, (1 << 5)    ; CR4.PAE
    mov cr4, eax

.enable64BitMode:
    BiosPrintMacro String.Status.Entering64Bit

    ; Enable 64-bit mode
    mov ecx, 0xc0000080 ; Extended Feature Enable Register (EFER)
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging and protected mode.
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)    ; CR0.PG, CR0.PE
    mov cr0, eax

    ; Do a long jump using the new GDT, which forces the switch to 64-bit
    ; mode.
    jmp GDT64.Selector.Kernel.Code : .launch64

bits 64

.launch64:
    ; Set up the data segment registers.
    mov ax, GDT64.Selector.Kernel.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set the kernel stack pointer.
    mov rsp, Mem.Kernel.Stack.Top

    ; Initialize all general purpose registers.
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rdi, rdi
    xor rsi, rsi
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

    ; Do a jump to the kernel's entry point.
    push dword boot_info
    jmp Mem.Kernel.Code + (load - 0x7E00) ; wierd elf bug

bits 16

error:
    jmp .hang

.a20line:
    BiosPrintMacro String.Error.A20
    jmp .hang

.noCPUID:
    BiosPrintMacro String.Error.NoCPUID
    jmp .hang

.no64BitMode:
    BiosPrintMacro String.Error.No64BitMode
    jmp .hang

.noFXinst:
    BiosPrintMacro String.Error.NoFXinst
    jmp .hang

.noSSE:
    BiosPrintMacro String.Error.NoSSE
    jmp .hang

.undetectedMemory
    BiosPrintMacro String.Error.UndetectedMemory
    jmp .hang

.kernelNotFound:
    BiosPrintMacro String.Error.KernelNotFound
    jmp .hang

.kernelLoadFailed:
    BiosPrintMacro String.Error.KernelLoadFailed
    jmp .hang

.hang:
    cli
    hlt
    jmp .hang

%include "include/bootinfo.inc"
boot_info:
istruc bootinfo
    at bootinfo.memoryMapAddress,   dd 0
    at bootinfo.memoryMapEntries,   dd 0
    at bootinfo.memorySizeLow,      dd 0
    at bootinfo.memorySizeHigh,     dd 0
    at bootinfo.bootDevice,         dd 0
iend

partition_offset dw 0x0000
ImageName db "KERNEL  SYS"

String.Loading  db "Loading...", 13, 10, 0

String.Status.A20Enabled        db "A20 line enabled", 13, 10, 0
String.Status.CPU64Detected     db "64-bit CPU detected", 13, 10, 0
String.Status.SSEEnabled        db "SSE enabled", 13, 10, 0
String.Status.DetectMemory      db "Memory detected", 13, 10, 0
String.Status.KernelLoaded      db "Kernel loaded", 13, 10, 0
String.Status.Entering64Bit     db "Entering 64 bit", 13, 10, 0

String.Error.A20                db "A20 line not enabled", 13, 10, 0
String.Error.NoCPUID            db "CPUID not supported", 13, 10, 0
String.Error.No64BitMode        db "CPU is not 64-bit", 13, 10, 0
String.Error.NoFXinst           db "No FXSAVE/FXRSTOR", 13, 10, 0
String.Error.NoSSE              db "No SSE support", 13, 10, 0
String.Error.UndetectedMemory  db "Error detecting memory", 13, 10, 0
String.Error.KernelNotFound     db "Kernel not found", 13, 10, 0
String.Error.KernelLoadFailed   db "Kernel load failed", 13, 10, 0

%define SECOND_STAGE
%include "include/memory_layout.inc"
%include "include/bios_print.inc"
%include "include/a20.inc"
%include "include/cpuid.inc"
%include "include/gdt.inc"
%include "include/paging.inc"
%include "include/sse.inc"
%include "include/fat16_bpb.inc"
%include "include/fat16.inc"
%include "include/fat16_ext.inc"
%include "include/memory.inc"

bits 16

LoadKernel:
    call RetriveBPB
    call LoadRoot       ; Load FAT16 root directory
    mov ebx, Mem.Kernel.Image
    mov si, ImageName
    call LoadFileExt
    cmp ax, 0
    je .done
    stc                 ; sets eror
.done
    ret

bpb_Loader1 equ Mem.Loader1 + 3 ; + 3 to skip the jump instruction
RetriveBPB:
    mov eax, dword [bpb_Loader1 + 00 + 0]
    mov dword [pbp_OEM + 0], eax
    mov eax, dword [bpb_Loader1 + 00 + 4]
    mov dword [pbp_OEM + 4], eax

    mov ax, word [bpb_Loader1 + 08]
    mov word [bpb_BytesPerSector], ax

    mov al, byte [bpb_Loader1 + 10]
    mov byte [bpb_SectorsPerCluster], al

    mov ax, word [bpb_Loader1 + 11]
    mov word [bpb_ReservedSectors], ax

    mov al, byte [bpb_Loader1 + 13]
    mov byte [bpb_NumberOfFATs], al

    mov ax, word [bpb_Loader1 + 14]
    mov word [bpb_RootEntries], ax

    mov ax, word [bpb_Loader1 + 16]
    mov word [bpb_TotalSectors], ax

    mov al, byte [bpb_Loader1 + 18]
    mov byte [bpb_Media], al

    mov ax, word [bpb_Loader1 + 19]
    mov word [bpb_SectorsPerFAT], ax

    mov ax, word [bpb_Loader1 + 21]
    mov word [bpb_SectorsPerTrack], ax

    mov ax, word [bpb_Loader1 + 23]
    mov word [bpb_HeadsPerCylinder], ax

    mov eax, dword [bpb_Loader1 + 25]
    mov dword [bpb_HiddenSectors], eax

    mov eax, dword [bpb_Loader1 + 29]
    mov dword [bpb_TotalSectorsBig], eax

    mov al, byte [bpb_Loader1 + 33]
    mov byte [bpb_DriveNumber], al

    mov al, byte [bpb_Loader1 + 34]
    mov byte [bpb_Unused], al

    mov al, byte [bpb_Loader1 + 35]
    mov byte [bpb_ExtBootSignature], al

    mov eax, dword [bpb_Loader1 + 36]
    mov dword [bpb_SerialNumber], eax

    mov eax, dword [bpb_Loader1 + 40 + 0]
    mov dword [bpb_SerialNumber + 0], eax
    mov eax, dword [bpb_Loader1 + 40 + 4]
    mov dword [bpb_SerialNumber + 4], eax
    mov eax, dword [bpb_Loader1 + 40 + 8]
    mov dword [bpb_SerialNumber + 8], eax

    mov eax, dword [bpb_Loader1 + 51 + 0]
    mov dword [bpb_FileSystem + 0], eax
    mov eax, dword [bpb_Loader1 + 51 + 4]
    mov dword [bpb_FileSystem + 4], eax

    ret