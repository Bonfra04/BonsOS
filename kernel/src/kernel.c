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
#include <io/keyboard/keyboard.h>

#include <interrupts/pic.h>
#include <interrupts/idt.h>
#include <interrupts/exceptions.h>
#include <interrupts/ioapic.h>
#include <interrupts/lapic.h>

#include <drivers/pci.h>
#include <drivers/storage/ata.h>
#include <drivers/usb/usb.h>
#include <drivers/storage/mass_storage.h>

#include <storage/storage.h>

#include <fsys/fsys.h>
#include <fsys/fat16/fat16.h>
#include <fsys/vfs/vfs_tty.h>
#include <fsys/vfs/vfs_fb.h>

#include <smp/scheduler.h>

#include <syscall/syscall.h>

#include <executable/executable.h>

#include <timers/hpet.h>
#include <timers/rtc.h>

#include <log.h>

#include <linker.h>

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bootinfo.h"

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

    // hpet timer
    hpet_init();

    // rtc timer
    rtc_init();

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
    // pci drivers
    ata_init();

    // storage abstraction
    storage_init();

    // file system
    fsys_init();
    fsys_register(fat16_instantiate, PART_TYPE_FAT16);
    fsys_auto_mount();

    // reload default font
    text_renderer_load_font("a:/assets/zap-vga16.psf");

    // multi process
    scheduler_init();
    scheduler_start();

    // syscalls
    syscall_init();
    syscall_enable();

    // virtual file systems
    vfs_tty_init();
    vfs_fb_init();
}

void main(const bootinfo_t* bootinfo)
{
    init(bootinfo);
    kernel_log("BonsOS successfully booted\n");

    executable_t* executable = executable_load("a:/bin/init.elf");
    if(!executable)
        kernel_panic("Couldn't load init process");

    scheduler_run_executable(executable, "a:/", NULL, NULL);

    scheduler_terminate_thread();
}
