%ifndef __MEMORY_LAYOUT_INC__
%define __MEMORY_LAYOUT_INC__

%define LIN_TO_FAR_ADDR(linaddr) (((linaddr >> 4) << 16) | (linaddr & 0xf))

Mem.BIOS.IVT                        equ 0x00000000
Mem.BIOS.Data                       equ 0x00000400
Mem.VESA.Info                       equ 0x00000500
Mem.VESA.ModeInfo                   equ 0x00000700
Mem.MemoryMap                       equ 0x00000800
Mem.GDT                             equ 0x00004500
Mem.Loader.Stack.Bottom             equ 0x00004600
Mem.Loader.Stack.Top                equ 0x00007A00
Mem.MBR                             equ 0x00007A00
Mem.Loader1                         equ 0x00007C00
Mem.Loader2                         equ 0x00007E00

Mem.PageTable                       equ 0x00010000
Mem.PageTable.PML4T                 equ 0x00010000
Mem.PageTable.PDPT                  equ 0x00011000
Mem.PageTable.PDT                   equ 0x00012000  ; maps first 10MiB
Mem.PageTable.PT                    equ 0x00013000  ; maps first 2MiB
Mem.PageTable.End                   equ 0x00020000

Mem.Loader.FAT16EXT.LoadBuffer      equ 0x00010000
Mem.Loader.FAT16.Data               equ 0x00010800

Mem.BIOS.ExtendedData               equ 0x0009FC00
Mem.Video                           equ 0x000A0000
Mem.BIOS                            equ 0x000C0000

Mem.Kernel.Image                    equ 0x00100000
Mem.Kernel.Code                     equ 0x00101000
Mem.Kernel.Stack.Bottom             equ 0x00C00000
Mem.Kernel.Stack.Top                equ 0x00D00000

%endif ; __MEMORY_LAYOUT_INC__