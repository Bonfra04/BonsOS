#include <acpi.h>
#include <panic.h>
#include <log.h>
#include <memory/paging.h>

#include <linker.h>

#include <string.h>

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

void acpi_init()
{
    find_rsdt();
}

void* acpi_find_entry(const char signature[4])
{
    size_t num_entries = (rsdt->length - sizeof(rsdt_header_t)) / 4;
    uint32_t* entries = (uint32_t*)(rsdt + 1);

    for(size_t i = 0; i < num_entries; i++)
    {
        rsdt_header_t* header = (rsdt_header_t*)(uint64_t)entries[i];
        paging_map(NULL, header, header, header->length, PAGE_PRIVILEGE_KERNEL);

        if(memcmp(header->signature, signature, 4) == 0)
            return header;
    }

    return NULL;
}
