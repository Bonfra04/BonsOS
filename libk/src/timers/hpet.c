#include <timers/hpet.h>
#include <memory/paging.h>
#include <memory/pfa.h>
#include <acpi.h>

typedef struct hpet_address
{
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed)) hpet_address_t;

typedef struct hpet_header
{
    rsdt_header_t header;
    uint8_t hardware_rev_id;
    uint8_t flags;
    uint16_t pci_vendor_id;
    hpet_address_t address;
    uint8_t hpet_number;
    uint16_t minimun_tick;
    uint8_t page_protection;
} __attribute__((packed)) hpet_header_t;

#define HPET_REG_CAPS_ID    0x00
#define HPET_REG_CONFIG     0x10
#define HPET_REG_COUNTER    0xF0
#define HPET_REG_CONF0    0x100
#define HPET_REG_COMP0    0x108

#define HPET_CNF_ENABLE     1

#define kFemtosPerNano 1000000

typedef struct hpet_regs
{
    uint64_t cap;
    uint64_t reserved_0;
    uint64_t config;
    uint64_t reserved_1;
    uint64_t irq_status;
    uint8_t reserved_2[0xC8];
    uint64_t main_counter;
    uint64_t reserved_3;
    struct
    {
        uint64_t config;
        uint64_t value;
        uint64_t fsb;
        uint64_t reserved;
    } __attribute__((packed)) comparators[32];
} __attribute__((packed)) hpet_regs_t;

static hpet_regs_t* hpet_regs;
static uint64_t period;

void hpet_init()
{
    hpet_header_t* hpet = acpi_find_entry("HPET");

    hpet_regs = (hpet_regs_t*)hpet->address.address;
    paging_map(kernel_paging, hpet_regs, hpet_regs, sizeof(hpet_regs_t), PAGE_PRIVILEGE_KERNEL);

    // get period in femtoseconds
    period = hpet_regs->cap >> 32;

    // disable all timers
    for(int i = 0; i < 32; i++)
    {
        hpet_regs->comparators[i].config &= ~(1 << 2); // is INT_ENB_CNF
        hpet_regs->comparators[i].value = 0;
    }

    // enable main counter
    hpet_regs->config |= HPET_CNF_ENABLE;
}

uint64_t hpet_current_nanos()
{
    return hpet_regs->main_counter * (period / kFemtosPerNano);
}
