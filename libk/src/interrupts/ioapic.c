#include <interrupts/ioapic.h>
#include <acpi.h>
#include <memory/heap.h>
#include <interrupts/lapic.h>

#include <linker.h>
#include <log.h>

#define IOAPICID    0x0
#define IOAPICVER   0x1
#define IOAPICARB   0x2

#define DELV_FIXED  0b000
#define DELV_LOW    0b001
#define DELV_SMI    0b010
#define DELV_NMI    0b100
#define DELV_INIT   0b101
#define DELV_EXTINT 0b111

#define DEST_PHYS   0
#define DEST_LOG    1

#define PIN_AHIGH   0
#define PIN_ALOW    1

#define TRIG_EDGE   0
#define TRIG_LEVEL  1

typedef struct ioapic
{
    uint32_t address;
    uint32_t gsi_base;
    uint8_t num_entries;
    uint8_t id;
} ioapic_t;

typedef union redirection_entry
{
    uint64_t raw;
    struct
    {
        uint64_t vector       : 8;
        uint64_t delvMode     : 3;
        uint64_t destMode     : 1;
        uint64_t delvStatus   : 1;
        uint64_t pinPolarity  : 1;
        uint64_t remoteIRR    : 1;
        uint64_t triggerMode  : 1;
        uint64_t mask         : 1;
        uint64_t reserved     : 39;
        uint64_t destination  : 8;
    } __attribute__ ((packed));
} redirection_entry_t;

static ioapic_t* ioapics;
static size_t num_ioapic;

static void ioapic_write(const void* base, uint8_t offset, uint32_t val)
{
    *(volatile uint32_t*)((uint64_t)base) = offset;
    *(volatile uint32_t*)((uint64_t)base + 0x10) = val;
}

static uint32_t ioapic_read(const void* base, uint32_t offset)
{
    *(volatile uint32_t*)((uint64_t)base) = offset;
    return *(volatile uint32_t*)((uint64_t)base + 0x10);
}

static uint64_t ioapic_read_irq(const void* base, uint8_t irq)
{
    uint32_t offset = 0x10 + irq * 2;

    uint32_t low = ioapic_read(base, offset);
    uint32_t high = ioapic_read(base, offset + 1);

    return ((uint64_t)high << 32) | low;
}

static void ioapic_write_irq(const void* base, uint8_t irq, uint64_t entry)
{
    uint32_t offset = 0x10 + irq * 2;

    uint32_t low = entry & 0xFFFFFFFF;
    uint32_t high = entry >> 32;

    ioapic_write(base, offset, low);
    ioapic_write(base, offset + 1, high);
}

void ioapic_init()
{
    madt_data_t madt_data = acpi_get_madt();

    linked_list_t ioapic_entries = madt_data.ioapic_entries;
    num_ioapic = linked_list_size(ioapic_entries);
    ioapics = heap_malloc(sizeof(ioapic_t) * num_ioapic);

    for(size_t i = 0; i < num_ioapic; i++)
    {
        ioapic_entry_t* ioapic_entry = linked_list_val(ioapic_entries, ioapic_entry_t*, i);

        ioapics[i].address = ioapic_entry->ioapic_address;
        ioapics[i].gsi_base = ioapic_entry->gsi_base;
        ioapics[i].num_entries = (ioapic_read(ptr(ioapics[i].address), IOAPICVER) >> 16) + 1;
        ioapics[i].id = ioapic_entry->ioapic_id;

        for(uint16_t red = 0; red < ioapics[i].num_entries; red++)
        {
            redirection_entry_t entry;
            entry.raw = 0;

            entry.vector = ioapics[i].gsi_base + red + IRQ_OFFSET;
            entry.delvMode = DELV_FIXED;
            entry.destMode = DEST_PHYS;
            entry.delvStatus = 0;
            entry.pinPolarity = PIN_AHIGH;
            entry.remoteIRR = 0; // TODO: tf is this
            entry.triggerMode = TRIG_EDGE;
            entry.mask = 1;
            entry.destination = lapic_get_boot_id();

            ioapic_write_irq(ptr(ioapics[i].address), red, entry.raw);
        }
    }

    linked_list_t overrides = madt_data.ioapic_int_src_ovr_entries;
    size_t num_overrides = linked_list_size(overrides);
    
    for(size_t i = 0; i < num_overrides; i++)
    {
        ioapic_int_src_ovr_entry_t* override = linked_list_val(overrides, ioapic_int_src_ovr_entry_t*, i);

        for(size_t ioapic = 0; ioapic < num_ioapic; ioapic++)
        {
            if(ioapics[i].gsi_base <= override->irq_source && ioapics[i].gsi_base + ioapics[i].num_entries > override->irq_source)
            {
                uint8_t irq = override->irq_source - ioapics[i].gsi_base;
                redirection_entry_t entry;
                entry.raw = ioapic_read_irq(ptr(ioapics[i].address), irq);

                entry.vector = override->gsi + IRQ_OFFSET;

                entry.pinPolarity = (override->flags & 2) ? PIN_ALOW : PIN_AHIGH;
                entry.triggerMode = (override->flags & 8) ? TRIG_LEVEL : TRIG_EDGE;

                ioapic_write_irq(ptr(ioapics[i].address), irq, entry.raw);
            }
        }
    }
}

void ioapic_unmask(uint8_t irq)
{
    for(size_t i = 0; i < num_ioapic; i++)
    {
        for(uint16_t red = 0; red < ioapics[i].num_entries; red++)
        {
            redirection_entry_t entry;
            entry.raw = ioapic_read_irq(ptr(ioapics[i].address), red);

            if(entry.vector == irq + IRQ_OFFSET)
            {
                entry.mask = 0;
                ioapic_write_irq(ptr(ioapics[i].address), red, entry.raw);
                return;
            }
        }
    }
}

void ioapic_mask(uint8_t irq)
{
    for(size_t i = 0; i < num_ioapic; i++)
    {
        for(uint16_t red = 0; red < ioapics[i].num_entries; red++)
        {
            redirection_entry_t entry;
            entry.raw = ioapic_read_irq(ptr(ioapics[i].address), red);

            if(entry.vector == irq + IRQ_OFFSET)
            {
                entry.mask = 1;
                ioapic_write_irq(ptr(ioapics[i].address), red, entry.raw);
                return;
            }
        }
    }
}

void ioapic_eoi()
{
    lapic_eoi();
}
