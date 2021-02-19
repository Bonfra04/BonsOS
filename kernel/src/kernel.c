#include <device/tty.h>
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <device/keyboard.h>
#include <memory/memory_map.h>
#include <memory/page_frame_allocator.h>
#include <memory/heap.h>
#include <device/pci.h>
#include <device/ata.h>
#include <device/pit.h>
#include <device/ata/sata.h>
#include <x86/cpu.h>
#include <memory/paging_test.h>

#include <string.h>
#include <stdlib.h>


#include "bootinfo.h"

heap_data_t kernel_heap;

void init(bootinfo_t* bootinfo)
{
    tty_init();
    
    interrupts_init();
    exceptions_init();

    kb_init();
    //pit_initialize();
    //pit_reset_counter(100, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);

    enable_interrupts();

    identity_map_everything();

    // First MB + number of KB above 1MB + 64 * number of 64KB blocks above 16MB
    memory_map_init(bootinfo->memoryMapEntries, (void*)(uint64_t)bootinfo->memoryMapAddress, 1024 + (uint64_t)bootinfo->memorySizeLow + (uint64_t)bootinfo->memorySizeHigh * 64ull);
    // After the stack
    pfa_init((void*)0x00D00001);
    // Deinit the region the kernel/stack is in as its in use
    uint64_t size = pfa_get_bitmap_size();
    pfa_deinit_region(0, 0x00D00000 + size);
    
    kernel_heap = heap_create(pfa_alloc_page(), pfa_get_page_size());
    heap_activate(&kernel_heap);

    pci_init();

    int bits = sizeof(void*) * 8;
    tty_printf("Succesfully booted BonsOS %d bit\n", bits);
}

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    /*
    uint8_t disk[512];
    bool success = sata_read(0, 0, 1, (void*)&disk);

    tty_printf("%d\n", success);
    for(int i = 0; i < 512; i++)
        tty_printf("%X", disk[i]);
    tty_printf("\n");
    */

    while(true)
    {
        char buff[32];
        tty_set_textcolor_fg(TEXTCOLOR_LTBLUE);
        tty_printf("BonsOS ");
        tty_set_textcolor_fg(TEXTCOLOR_YELLOW);
        tty_printf("$> ");
        tty_set_textcolor_fg(TEXTCOLOR_WHITE);
        tty_scanf("%s", buff);
        
        if(strcmp(buff, "exit") == 0)
            break;
    }
}
