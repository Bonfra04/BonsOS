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

#include "paging_tmp.h"
#include "bootinfo.h"

#include "unit_tests/unit_test.h"

void init(bootinfo_t* bootinfo)
{
    identity_map_everything();

    screen_init(bootinfo->screen_width, bootinfo->screen_height, bootinfo->screen_pitch, (void*)(uint64_t)bootinfo->framebuffer);
    tty_init();

    interrupts_init();
    exceptions_init();

    kb_init();

    enable_interrupts();

    // First MB + number of KB above 1MB + 64 * number of 64KB blocks above 16MB
    uint64_t memorySize = 1024 + (uint64_t)bootinfo->memorySizeLow + (uint64_t)bootinfo->memorySizeHigh * 64ull;
    memory_map_init(bootinfo->memoryMapEntries, (void*)(uint64_t)bootinfo->memoryMapAddress, memorySize);
    // After the stack
    pfa_init((void*)0x00D00001);
    // Deinit the region the kernel/stack/bitmap is in as its in use
    pfa_deinit_region(0, 0x00D00000 + pfa_get_bitmap_size());

    static heap_data_t kernel_heap;
    kernel_heap = heap_create(pfa_alloc_page(), pfa_page_size());
    heap_activate(&kernel_heap);

    pci_init();

    disk_manager_init();

    renderer_load_font("a:/fonts/zapvga16.psf");

    pit_initialize();
    pit_reset_counter(1, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);

    scheduler_initialize();

    int bits = sizeof(void*) * 8;
    printf("Succesfully booted BonsOS %d bit.\n", bits);

    static paging_data_t paging_data;
    paging_data = paging_create();
    for(uint64_t i = 0; i < memorySize / 1024; i += 2)
        paging_attach_2mb_page(paging_data, i * 0x200000, i * 0x200000);

    asm volatile("mov cr3, %[addr]" : : [addr]"r"(paging_data) : "memory");
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

typedef char symbol[];

void* kernel_start;
void* kernel_end;

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    if(!execute_tests())
        return;

    extern symbol __kernel_start_addr;
    extern symbol __kernel_end_addr;
    kernel_start = __kernel_start_addr;
    kernel_end = __kernel_end_addr;

    create_process(shell, PRIVILEGE_KERNEL);
    schedule();

    while(1)
        asm("pause");
}
