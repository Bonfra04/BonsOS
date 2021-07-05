#include <device/tty.h>
#include <graphics/screen.h>
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <device/keyboard.h>
#include <memory/memory_map.h>
#include <memory/page_frame_allocator.h>
#include <memory/heap.h>
#include <device/pci.h>
#include <device/ata.h>
#include <device/ata/ahci.h>
#include <x86/cpu.h>
#include <filesystem/fat16.h>
#include <storage/disk_manager.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <filesystem/fsys.h>
#include <filesystem/ttyfs.h>
#include <graphics/renderer.h>
#include <schedule/scheduler.h>
#include <device/pit.h>
#include <memory/paging.h>
#include <x86/gdt.h>
#include <syscall/syscall.h>
#include <executable/executable.h>
#include <filesystem/pipefs.h>

#include "bootinfo.h"
#include "dbg/dbg.h"

#include "unit_tests/unit_test.h"

typedef struct system_info
{
    void* framebuffer;
    size_t screen_width;
    size_t sreen_height;
    size_t screen_pitch;
} system_info_t;

typedef char symbol[];

paging_data_t kernel_paging;
heap_data_t kernel_heap;

static system_info_t system_info;

void init(bootinfo_t* bootinfo)
{
    // zero out bss
    extern symbol __bss_start_addr, __bss_size;
    memset((void*)__bss_start_addr, 0, (size_t)__bss_size);

    extern void initialize_standard_library();
    initialize_standard_library();

    gdt_init();

    // First MB + number of KB above 1MB + 64 * number of 64KB blocks above 16MB
    uint64_t memorySize = 1024 + (uint64_t)bootinfo->memorySizeLow + (uint64_t)bootinfo->memorySizeHigh * 64ull;
    memory_map_init(bootinfo->memoryMapEntries, (void*)(uint64_t)bootinfo->memoryMapAddress, memorySize);
    // After the stack
    pfa_init((void*)0x00D00001);
    // Deinit the region the kernel/stack/bitmap is in as its in use
    pfa_deinit_region(0, 0x00D00000 + pfa_get_bitmap_size());

    size_t mem_2mball_size = (memorySize * 1024) + 0x200000 - (memorySize * 1024) % 0x200000;
    kernel_paging = paging_init(mem_2mball_size);

    kernel_heap = heap_create(pfa_alloc_page(), pfa_page_size());
    heap_activate(&kernel_heap);

    screen_init(bootinfo->screen_width, bootinfo->screen_height, bootinfo->screen_pitch, (void*)(uint64_t)bootinfo->framebuffer);
    renderer_init();

    interrupts_init();
    exceptions_init();

    tty_init();
    kb_init();

    enable_interrupts();

    dbg_init();

    pci_init();

    disk_manager_init();

    pipefs_mount(0);

    renderer_load_font("a:/fonts/zapvga16.psf");

    pit_initialize();
    pit_reset_counter(1, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);

    scheduler_initialize();

    syscall_init();

    system_info.framebuffer = bootinfo->framebuffer;
    system_info.screen_width = bootinfo->screen_width;
    system_info.sreen_height = bootinfo->screen_height;
    system_info.screen_pitch = bootinfo->screen_pitch;

    int bits = sizeof(void*) * 8;
    printf("Succesfully booted BonsOS %d bit.\n", bits);
}

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    if(!execute_tests())
        return;

    char fb[16], sw[16], sh[16], sp[16];
    ulltoa(system_info.framebuffer, fb, 16);
    ulltoa(system_info.screen_width, sw, 16);
    ulltoa(system_info.sreen_height, sh, 16);
    ulltoa(system_info.screen_pitch, sp, 16);
    char* argv[] = { fb, sw, sh, sp };

    size_t pid = run_executable("a:/bin/init.elf", 4, argv, ELF);

    if(pid == 0)
    {
        printf("Fatal error while loading core components.\n");
        return;
    }

    printf("Running OS...\n");
    schedule();
}
