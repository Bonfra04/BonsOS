#include <device/tty.h>
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <device/keyboard.h>
#include <memory/memory_map.h>
#include <memory/phys_mem_manager.h>
#include <memory/heap.h>
#include <device/pci.h>
#include <device/ata.h>
#include <device/pit.h>
#include <device/ata/sata.h>

#include <string.h>
#include <stdlib.h>

#include <x86/cpu.h>

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

    // First MB + number of KB above 1MB + 64 * number of 64KB blocks above 16MB
    memory_map_init(bootinfo->memoryMapEntries, (void*)0x500, 1024 + (uint64_t)bootinfo->memorySizeLow + (uint64_t)bootinfo->memorySizeHigh * 64ull);
    // After the stack
    pmm_init((void*)0x00300001);
    // Deinit the region the kernel/stack is in as its in use
    pmm_deinit_region(0, 0x00300000);
    
    kernel_heap = heap_create(pmm_alloc_block(), pmm_get_block_size());
    heap_activate(&kernel_heap);

    pci_init();

    int bits = sizeof(void*) * 8;
    tty_printf("Succesfully booted BonsOS %d bit\n", bits);
}

void main(bootinfo_t* bootinfo)
{
    init(bootinfo);

    while(true)
    {
        char buff[32];
        tty_printf("BonsOS $> ");
        tty_scanf("%s", buff);
        
        if(strcmp(buff, "exit") == 0)
            break;
    }
}