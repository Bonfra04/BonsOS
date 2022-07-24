#include <acpi.h>
#include <panic.h>
#include <log.h>
#include <memory/paging.h>

#include <linker.h>

#include <string.h>

#include "tables.h"

typedef struct rsdp_descriptor
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__ ((packed)) rsdp_descriptor_t;

typedef struct rsdp_descriptor_20
{
    rsdp_descriptor_t first_part;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) rsdp_descriptor_20_t;

static rsdt_header_t* rsdt;

static void* madt_lapic_address = NULL;
static lapic_entry_t* madt_lapic_entries;
static ioapic_entry_t* madt_ioapic_entries;
static ioapic_int_src_ovr_entry_t* madt_int_src_ovr_entries;

static void find_rsdt()
{
    rsdp_descriptor_t* rsdp = NULL;

    for(uint64_t addr = 0xE0000; addr < 0xFFFFF; addr += 16)
        if(memcmp((void*)addr, "RSD PTR ", 8) == 0)
        {
            rsdp = (rsdp_descriptor_t*)addr;
            break;
        }

    // TODO: check extended bios area

    if(rsdp == NULL)
        kernel_panic("find_rsdp: ACPI RSDP not found");

    paging_map(NULL, rsdp, rsdp, sizeof(rsdp_descriptor_20_t), PAGE_PRIVILEGE_KERNEL);

    rsdt = (rsdt_header_t*)(rsdp->revision == 0 ? rsdp->rsdt_address : ((rsdp_descriptor_20_t*)rsdp)->xsdt_address);

    paging_map(NULL, rsdt, rsdt, rsdt->length, PAGE_PRIVILEGE_KERNEL);
}

static void parse_apic_table(const rsdt_header_t* header);

static void parse_tables()
{
    size_t num_entries = (rsdt->length - sizeof(rsdt_header_t)) / 4;
    uint32_t* entries = (uint32_t*)(rsdt + 1);

    for(size_t i = 0; i < num_entries; i++)
    {
        rsdt_header_t* header = (rsdt_header_t*)(uint64_t)entries[i];
        paging_map(NULL, header, header, header->length, PAGE_PRIVILEGE_KERNEL);

        if(memcmp(header->signature, "APIC", 4) == 0)
            parse_apic_table(header);
    }
}

void acpi_init()
{
    madt_lapic_entries = darray(lapic_entry_t, 0);
    madt_ioapic_entries = darray(ioapic_entry_t, 0);
    madt_int_src_ovr_entries = darray(ioapic_int_src_ovr_entry_t, 0);

    find_rsdt();
    parse_tables();
}

static void parse_apic_table(const rsdt_header_t* header)
{
    madt_header_t* madt = (madt_header_t*)header;
    madt_entry_t* entry = (madt_entry_t*)(madt + 1);

    madt_lapic_address = ptr(madt->lapic_address);

    for(size_t len = sizeof(madt_header_t); len < madt->header.length; len += entry->record_lenght)
    {
        switch (entry->entry_type)
        {
        case MADT_LAPIC:
        {
            lapic_entry_t* lapic = (lapic_entry_t*)(entry + 1);
            if(lapic->flags & 1 || lapic->flags & 2)
                darray_append(madt_lapic_entries, *lapic);
            break;
        }

        case MADT_IOAPIC:
        {
            ioapic_entry_t* ioapic = (ioapic_entry_t*)(entry + 1);
            darray_append(madt_ioapic_entries, *ioapic);
            break;
        }

        case MADT_INT_SRC_OVR:
        {
            ioapic_int_src_ovr_entry_t* int_src_ovr = (ioapic_int_src_ovr_entry_t*)(entry + 1);
            darray_append(madt_int_src_ovr_entries, *int_src_ovr);
            break;
        }

        case MADT_NMI_SRC:
            kernel_warn("Ignoring MADT NMI source");
            break;

        case MADT_LAPIC_NMI:
            kernel_warn("Ignoring MADT LAPIC NMI");
            break;

        case MADT_LAPIC_OVR:
        {
            lapic_addr_ovr_entry_t* lapic_addr_ovr = (lapic_addr_ovr_entry_t*)(entry + 1);
            madt_lapic_address = (void*)lapic_addr_ovr->address;
            break;
        }

        case MADT_Lx2APIC:
            kernel_warn("Ignoring MADT Lx2APIC");
            break;

        }
        entry = (madt_entry_t*)((uint8_t*)entry + entry->record_lenght);
    }
}

madt_data_t acpi_get_madt()
{
    return (madt_data_t)
    {
        .lapic_address = madt_lapic_address,
        .lapic_entries = madt_lapic_entries,
        .ioapic_entries = madt_ioapic_entries,
        .ioapic_int_src_ovr_entries = madt_int_src_ovr_entries,
    };
}
