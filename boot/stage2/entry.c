#include "sys/a20.h"
#include "sys/cpuid.h"
#include "sys/sse.h"
#include "sys/vesa.h"
#include "sys/tty.h"
#include "sys/fat16.h"
#include "sys/memory.h"
#include "sys/paging.h"
#include "sys/gdt.h"
#include "sys/disk.h"

#include "lib/stdint.h"
#include "lib/stdbool.h"
#include "lib/string.h"

#include "bootinfo.h"

#define KERNEL_ADDRESS ((void*)0x00100000)

extern void lmode_enable();

bootinfo_t bootinfo;
size_t low_mem_size;

void main(uint8_t boot_drive, uint16_t partition_offset)
{
    tty_printf("Booting\n");

    if(!a20_enable())
    {
        tty_printf("Cannot enable A20\n");
        return;
    }

    if(cpuid_unsupported())
    {
        tty_printf("CPUID not supported\n");
        return;
    }

    char vendor[13];
    cpuid_vendor(vendor);
    tty_printf("CPU Vendor: %s\n", vendor);

    if(!cpuid_longmode_supported())
    {
        tty_printf("Long mode not supported\n");
        return;
    }

    if(!fx_supported() || !sse_supported())
    {
        tty_printf("SSE not supported\n");
        return;
    }
    sse_enable();

    vesa_descriptor_t vesa_descriptor;
    if(!vesa_find_mode(1024, 768, 32, &vesa_descriptor))
    {
        tty_printf("No compatible vesa mode\n");
        return;
    }

    if(!memory_read_map())
    {
        tty_printf("Cannot get memory map\n");
        return;
    }

    size_t mem_siz = memory_size() / (1024 * 1024) + (memory_size() % (1024 * 1024) != 0);
    low_mem_size = memory_lower_size();
    tty_printf("Detected %llu mib of memory\n", mem_siz);

    paging_identity_map();

    disk_init(boot_drive);
    fsys_init(partition_offset * 512);

    if(!file_read("KERNEL  SYS", KERNEL_ADDRESS))
    {
        tty_printf("Cannot load kernel\n");
        return;
    }

    // TODO: ZERO OUT KERNEL BSS

    tty_printf("Kernel loaded succesfully\n");

    vesa_enable_mode(vesa_descriptor.id);

    gdt_install();

    bootinfo.memoryMapAddress = &memory_map;
    bootinfo.memoryMapEntries = memory_num_entries();
    bootinfo.memory_size = mem_siz * 1024 * 1024;
    bootinfo.bootDevice = boot_drive;
    bootinfo.screen_width = vesa_descriptor.width;
    bootinfo.screen_height = vesa_descriptor.height;
    bootinfo.screen_pitch = vesa_descriptor.pitch;
    bootinfo.framebuffer = vesa_descriptor.framebuffer;

    lmode_enable();
}
