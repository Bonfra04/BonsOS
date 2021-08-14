#include <smp/smp.h>
#include <acpi/acpi.h>
#include <stddef.h>
#include <string.h>
#include <panic.h>
#include <x86/cpu.h>
#include <interrupt/apic.h>
#include <stdio.h>
#include <memory/paging.h>
#include <smp/scheduler.h>
#include <x86/gdt.h>
#include <smp/atomic.h>
#include <memory/page_frame_allocator.h>

#define TRAMPOLINE_ADDRESS 0x8000

typedef struct madt_entry
{
    uint8_t entry_type;
    uint8_t record_lenght;
} __attribute__((packed)) madt_entry_t;

typedef struct madt_descriptor
{
    sdt_header_t header;
    uint32_t lapic_address;
    uint32_t flags;
    madt_entry_t entries[]
} __attribute__((packed)) madt_descriptor_t;

typedef enum madt_entry_type
{
    LAPIC = 0,
    IOAPIC = 1,
    INT_SRC_OVR = 2,
    NMI_SRC = 3,
    LAPIC_NMI = 4,
    LAPIC_OVR = 5,
    Lx2APIC = 9,
} madt_entry_type_t;

typedef struct lapic_entry
{
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) lapic_entry_t;

typedef struct ap_data
{
    uint32_t kernel_pagign;
    uint64_t entry_point;
} __attribute__((packed)) ap_data_t;

static uint8_t lapic_ids[UINT8_MAX];
static uint8_t num_cores;
static uint8_t bsp_id;

static void detect_cores(const rsdt_decriptor_t* rsdt)
{
    size_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    for(size_t i = 0; i < entries; i++)
    {
        sdt_header_t* header = (sdt_header_t*)rsdt->addresses[i];
        if(!strncmp(header->signature, "APIC", 4))
        {
            madt_descriptor_t* madt = (madt_descriptor_t*)(header);
            madt_entry_t* entry = madt->entries;
            for(size_t len = sizeof(madt_descriptor_t); len < header->length; len += entry->record_lenght)
            {
                switch (entry->entry_type)
                {
                case LAPIC:
                {
                    lapic_entry_t* lapic = (lapic_entry_t*)((uint8_t*)entry + sizeof(madt_entry_t));
                    if(lapic->flags & 1) // processor enabled and online
                        lapic_ids[num_cores++] = lapic->apic_id;
                    break;
                }
                case LAPIC_OVR:
                    break;
                }

                entry = (uint8_t*)entry + entry->record_lenght;
            }
        }
    }
}

static bool core_flag;
static void core_entry()
{
    register void* stack = (uint8_t*)pfa_alloc_page() + pfa_page_size();
    asm("mov rsp, %0" :: "r"(stack));
    gdt_install();
    idt_install();
    scheduler_prepare();
    asm("sti");

    core_flag = true;

    apic_setup();
    scheduler_start(); // should never return
    while(1);
}

static void bootup_core(uint8_t core)
{
    if(core >= num_cores)
        return;

    uint8_t lapic_id = lapic_ids[core];
    if(lapic_id == bsp_id) // not initalize already running core
        return;

    FILE* pFile = fopen("a:/bin/smp_trmp.bin", "r");
    fseek(pFile, 0 , SEEK_END);
    size_t size = ftell(pFile);
    rewind(pFile);
    fread(TRAMPOLINE_ADDRESS, size, 1, pFile);

    extern paging_data_t kernel_paging;
    volatile ap_data_t* passed_data = (ap_data_t*)(TRAMPOLINE_ADDRESS + 2);
    passed_data->kernel_pagign = (uint32_t)kernel_paging;
    passed_data->entry_point = core_entry;
    core_flag = false;

    // send init ipi
    lapic_write(LAPIC_REG_ICR1, lapic_id << 24);
    lapic_write(LAPIC_REG_ICR0, 0x4500);

    port_delay(5000);

    // send startup ipi
    lapic_write(LAPIC_REG_ICR1, lapic_id << 24);
    lapic_write(LAPIC_REG_ICR0, 0x4600 | (TRAMPOLINE_ADDRESS / 0x1000));

    // wait for flag to be flipped
    while(core_flag == false)
        port_delay(10000);
}

void smp_init()
{
    num_cores = 0;
    memset(lapic_ids, 0, UINT8_MAX);
    detect_cores(acpi_rsdt());
    registers4_t regs;
    cpuid(1, &regs);
    bsp_id = regs.rbx >> 24;

    // if(num_cores < 2)
    //     kenrel_panic("2 cores or more are needed to run");

    for(uint8_t i = 0; i < num_cores; i++)
        bootup_core(i);
}

inline uint8_t smp_num_cores()
{
    return num_cores;
}