#include <acpi/acpi.h>
#include <string.h>
#include <panic.h>

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
    if(rsdp->revision != 0x0)
        kenrel_panic("Only ACPI 1.0 is supported");
}

static void find_rsdt()
{
    rsdt = (rsdt_decriptor_t*)rsdp->rsdt_address;
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
