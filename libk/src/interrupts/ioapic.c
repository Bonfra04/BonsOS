#include <interrupts/ioapic.h>
#include <acpi.h>
#include <interrupts/lapic.h>

#include <linker.h>
#include <log.h>

#include <stdlib.h>

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

typedef struct madt_entry
{
    uint8_t entry_type;
    uint8_t record_lenght;
} __attribute__ ((packed)) madt_entry_t;

typedef struct madt_header
{
    rsdt_header_t header;
    uint32_t lapic_address;
    uint32_t flags;
    madt_entry_t entries_begin[];
} __attribute__ ((packed)) madt_header_t;

typedef enum madt_entry_type
{
    MADT_IOAPIC = 1,
    MADT_INT_SRC_OVR = 2,
} madt_entry_type_t;

typedef struct ioapic_entry
{
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
} __attribute__((packed)) ioapic_entry_t;

typedef struct ioapic_int_src_ovr_entry
{
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) ioapic_int_src_ovr_entry_t;

static ioapic_t* ioapics;

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

static void register_ioapic(ioapic_entry_t* entry)
{
    ioapic_t ioapic;
    ioapic.address = entry->ioapic_address;
    ioapic.gsi_base = entry->gsi_base;
    ioapic.num_entries = (ioapic_read(ptr(ioapic.address), IOAPICVER) >> 16) + 1;
    ioapic.id = entry->ioapic_id;

    for(uint16_t red = 0; red < ioapic.num_entries; red++)
    {
        redirection_entry_t entry;
        entry.raw = 0;

        entry.vector = ioapic.gsi_base + red + IRQ_OFFSET;
        entry.delvMode = DELV_FIXED;
        entry.destMode = DEST_PHYS;
        entry.delvStatus = 0;
        entry.pinPolarity = PIN_AHIGH;
        entry.remoteIRR = 0; // TODO: tf is this
        entry.triggerMode = TRIG_EDGE;
        entry.mask = 1;
        entry.destination = lapic_get_boot_id();

        ioapic_write_irq(ptr(ioapic.address), red, entry.raw);
    }

    darray_append(ioapics, ioapic);
}

static void register_override(ioapic_int_src_ovr_entry_t* entry)
{
    for(size_t i = 0; i < darray_length(ioapics); i++)
    {
        if(ioapics[i].gsi_base <= entry->irq_source && ioapics[i].gsi_base + ioapics[i].num_entries > entry->irq_source)
        {
            uint8_t irq = entry->irq_source - ioapics[i].gsi_base;
            redirection_entry_t red;
            red.raw = ioapic_read_irq(ptr(ioapics[i].address), irq);

            red.vector = entry->gsi + IRQ_OFFSET;

            red.pinPolarity = (entry->flags & 2) ? PIN_ALOW : PIN_AHIGH;
            red.triggerMode = (entry->flags & 8) ? TRIG_LEVEL : TRIG_EDGE;

            ioapic_write_irq(ptr(ioapics[i].address), irq, red.raw);
        }
    }
}

void ioapic_init()
{
    ioapics = darray(ioapic_t, 0);

    madt_header_t* madt = acpi_find_entry("APIC");

    for(madt_entry_t* entry = madt->entries_begin; (uint8_t*)entry < (uint8_t*)madt + madt->header.length; entry = (madt_entry_t*)((uint8_t*)entry + entry->record_lenght))
    {
        switch (entry->entry_type)
        {
        case MADT_IOAPIC:
            register_ioapic((ioapic_entry_t*)(entry + 1));
            break;
        case MADT_INT_SRC_OVR:
            register_override((ioapic_int_src_ovr_entry_t*)(entry + 1));
            break;
        }
    }
}

void ioapic_unmask(uint8_t irq)
{
    for(size_t i = 0; i < darray_length(ioapics); i++)
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
    for(size_t i = 0; i < darray_length(ioapics); i++)
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
