#include <acpi/acpi.h>
#include <string.h>
#include <panic.h>
#include <memory/paging.h>

static rsdp_descriptor_t* rsdp;
static rsdt_decriptor_t* rsdt;

static void find_rsdp()
{
    rsdp = 0;

    // check main BIOS area
    for(uint64_t i = 0xE0000; i < 0xFFFFF; i += 16)
        if(memcmp(i, "RSD PTR ", 8) == 0)
        {
            rsdp = (rsdp_descriptor_t*)i;
            break;
        }

    // validate
    if(rsdp == 0)
        kenrel_panic("ACPI RSDP not found");

    paging_map_global(rsdp, rsdp, sizeof(rsdp_descriptor_20_t), PAGE_PRIVILEGE_KERNEL);
}

static void find_rsdt()
{
    if(rsdp->revision == 0x0) // acpi < 2.0
        rsdt = (rsdt_decriptor_t*)rsdp->rsdt_address;
    else // acpi >= 2.0
        rsdt = ((rsdp_descriptor_20_t*)rsdp)->xsdt_address;

    paging_map_global(rsdt, rsdt, sizeof(rsdt_decriptor_t), PAGE_PRIVILEGE_KERNEL);
}

void acpi_init()
{
    find_rsdp();
    find_rsdt();
}

inline const rsdp_descriptor_t* acpi_rsdp()
{
    return rsdp;
}

inline const rsdt_decriptor_t* acpi_rsdt()
{
    return rsdt;
}
