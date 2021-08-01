#include "gdt.h"

#include "../lib/stdint.h"
#include "../lib/string.h"

#define SELECTOR_KERNEL_CODE 0x08
#define SELECTOR_KERNEL_DATA 0x10

#define GDT_PRESENT (1 << 7)
#define GDT_TYPE (1 << 4)
#define GDT_EXEC (1 << 3)
#define GDT_READ_WRITE (1 << 1)

#define GDT_64BIT (1 << 5)

typedef struct gdt_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
} __attribute__ ((packed)) gdt_descriptor_t;

static gdt_descriptor_t __attribute__ ((aligned (16))) GDT[3];

static struct
{
    uint16_t size;
    uint64_t offset;
} __attribute__ ((packed)) __attribute__ ((aligned (16))) gdtr;

void gdt_install()
{
    memset(&GDT, 0, sizeof(GDT));

    // Kernel data descriptor
    GDT[SELECTOR_KERNEL_DATA / 8].access = GDT_PRESENT | GDT_TYPE | GDT_READ_WRITE;
    GDT[SELECTOR_KERNEL_DATA / 8].limit_high_flags = GDT_64BIT;

    // Kernel code descriptor
    GDT[SELECTOR_KERNEL_CODE / 8].access = GDT_PRESENT | GDT_TYPE | GDT_EXEC | GDT_READ_WRITE;
    GDT[SELECTOR_KERNEL_CODE / 8].limit_high_flags = GDT_64BIT;

    gdtr.size = sizeof(GDT) - 1;
    gdtr.offset = (uint32_t)&GDT;

    asm volatile("lgdt %[addr]" : : [addr]"m"(gdtr) : "memory");
}