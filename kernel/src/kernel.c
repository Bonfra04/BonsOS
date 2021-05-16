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

#include "paging_tmp.h"
#include "bootinfo.h"

#include "unit_tests/unit_test.h"

void init(bootinfo_t* bootinfo)
{
    identity_map_everything();

    screen_init(bootinfo->screen_width, bootinfo->screen_height, bootinfo->screen_pitch, bootinfo->framebuffer);
    tty_init();

    interrupts_init();
    exceptions_init();

    kb_init();

    enable_interrupts();

    // First MB + number of KB above 1MB + 64 * number of 64KB blocks above 16MB
    uint64_t memorySize = 1024 + (uint64_t)bootinfo->memorySizeLow + (uint64_t)bootinfo->memorySizeHigh * 64u;
    memory_map_init(bootinfo->memoryMapEntries, (void*)(uint64_t)bootinfo->memoryMapAddress, memorySize);
    // After the stack
    pfa_init((void*)0x00D00001);
    // Deinit the region the kernel/stack/bitmap is in as its in use
    pfa_deinit_region(0, 0x00D00000 + pfa_get_bitmap_size());

    static heap_data_t kernel_heap;
    kernel_heap = heap_create(pfa_alloc_page(), pfa_get_page_size());
    heap_activate(&kernel_heap);

    pci_init();

    disk_manager_init();

    renderer_load_font("a:/fonts/zapvga16.psf");

    pit_initialize();
    pit_reset_counter(200, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);

    scheduler_initialize();

    int bits = sizeof(void*) * 8;
    printf("Succesfully booted BonsOS %d bit.\n", bits);
}

void shell()
{
    while(true)
    {
        char buff[32];
        tty_set_textcolor_fg(0xFF0000FF); // blue
        printf("BonsOS ");
        tty_set_textcolor_fg(0xFFFFFF00); // yellow
        printf("$> ");
        tty_set_textcolor_fg(0xFFFFFFFF); // white
        scanf("%s", buff);

        if(strcmp("exit", buff) == 0)
            break;
    }

    disk_manager_flush(0);

    while(1);
}

void task1()
{
    while(1)
        printf("task1");
}

void task2()
{
    while(1)
        printf("task2");
}

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    if(!execute_tests())
        return;

    create_process(task1, PRIVILEGE_KERNEL);
    create_process(task2, PRIVILEGE_KERNEL);
    schedule();

    while(1)
        asm("pause");
}
