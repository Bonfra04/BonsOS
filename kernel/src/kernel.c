#include <device/tty.h>
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

#include "paging_tmp.h"
#include "bootinfo.h"

#include "unit_tests/unit_test.h"

void init(bootinfo_t* bootinfo)
{
    identity_map_everything();

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

    int bits = sizeof(void*) * 8;
    printf("Succesfully booted BonsOS %d bit.\n", bits);
}

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    if(!execute_tests())
        return;

    while(true)
    {
        char buff[32];
        tty_set_textcolor_fg(TEXTCOLOR_LTBLUE);
        printf("BonsOS ");
        tty_set_textcolor_fg(TEXTCOLOR_YELLOW);
        printf("$> ");
        tty_set_textcolor_fg(TEXTCOLOR_WHITE);
        gets(buff);
        
        if(strcmp("exit", buff) == 0)
            break;
    }

    disk_manager_flush(0);
}
