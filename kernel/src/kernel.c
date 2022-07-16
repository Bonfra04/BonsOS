#include <linker.h>
#include <panic.h>
#include <acpi.h>
#include <cpu.h>

#include <memory/gdt.h>
#include <memory/mmap.h>
#include <memory/pfa.h>
#include <memory/paging.h>
#include <memory/vmm.h>
#include <memory/heap.h>

#include <graphics/screen.h>
#include <graphics/text_renderer.h>

#include <io/uart.h>
#include <io/tty.h>
#include <io/keyboard.h>

#include <interrupts/pic.h>
#include <interrupts/idt.h>
#include <interrupts/exceptions.h>
#include <interrupts/ioapic.h>
#include <interrupts/lapic.h>

#include <pci/pci.h>

#include <storage/storage.h>

#include <fsys/fsys.h>
#include <fsys/fat16/fat16.h>
#include <log.h>

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bootinfo.h"

#include <pci/ata/sata.h>

void init(const bootinfo_t* bootinfo)
{
    // zero out bss
    extern symbol_t __bss_start_addr, __bss_size;
    memset((void*)__bss_start_addr, 0, (size_t)__bss_size); //  TODO: move to bootloader

    // install the gdt
    gdt_install();

    // initialize uart protocol
    uart_init();

    // initialize physical memory
    mmap_init(bootinfo->memoryMapEntries, ptr(bootinfo->memoryMapAddress), bootinfo->memory_size * 1024); // TODO: convert to bytes in the bootloader
    pfa_init();

    // initialize virtual memory
    paging_init();
    heap_init();

    // initialize graphics
    screen_init(bootinfo->screen_width, bootinfo->screen_height, bootinfo->screen_pitch, ptr(bootinfo->framebuffer));
    text_renderer_init();
    
    // initialize tty
    tty_init();

    // read the acpi tables
    acpi_init();

    // interrupts
    pic_disable();
    idt_init();
    idt_install();
    exceptions_init();
    lapic_init();
    lapic_setup();
    ioapic_init();
    sti();

    // keyboard and mouse
    keyboard_init();

    // pci devices
    pci_init();

    // storage abstraction
    storage_init();

    // file system
    fsys_init();
    fsys_register(fat16_instantiate, PART_TYPE_FAT16);
    fsys_auto_mount();
}

void main(const bootinfo_t* bootinfo)
{
    init(bootinfo);
    tty_print("BonsOS successfully booted\n");

    {
        fsys_create_dir("1:/test/giorgio");
    }

    {
        file_t f = fsys_open_file("1:/test/giorgio/mimmo.txt", FSYS_WRITE);
        fsys_write_file(&f, "dio cane mai", 12);
    }

    {
        file_t f = fsys_open_file("1:/test/giorgio/mimmo.txt", FSYS_WRITE);
        fsys_write_file(&f, "gioegio", 7);
    }

    {
        file_t f = fsys_open_dir("1:/test/giorgio");
        direntry_t e;
        while(fsys_list_dir(&f, &e))
            kernel_log("%s\n", e.name);
    }

    tty_print("All done\n");
    storage_flush(0);
    exit(EXIT_SUCCESS);
}
