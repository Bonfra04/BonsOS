#include <x86/gdt.h>
#include <stdint.h>
#include <string.h>

#define GDT_PRESENT (1 << 7)
#define GDT_PRIV_KERNEL (0 << 5)
#define GDT_PRIV_USER (3 << 5)
#define GDT_TYPE (1 << 4)
#define GDT_EXEC (1 << 3)
#define GDT_DIRECTION (1 << 2)
#define GDT_READ_WRITE (1 << 1)
#define GDT_ACCESSED (1 << 0)

#define GDT_64BIT (1 << 5)

#define TSS_PRESENT (1 << 7)
#define TSS_PRIV_KERNEL (0 << 5)
#define TSS_PRIV_USER (3 << 5)
#define TSS_FREE (0b1001 << 0)
#define TSS_BUSY (0b1011 << 0)

typedef struct gdtr
{
    uint16_t size;
    uint64_t offset;
} __attribute__ ((packed)) gdtr_t;

typedef struct gdt_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
} __attribute__ ((packed)) gdt_descriptor_t;

typedef struct tss_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
    uint32_t base_highest;
    uint32_t reserved;
} __attribute__ ((packed)) tss_descriptor_t;

typedef struct tss_entry
{
    uint32_t reserved0;
    uint64_t RSP0;
    uint64_t RSP1;
    uint64_t RSP2;
    uint64_t reserved1;
    uint64_t IST1;
    uint64_t IST2;
    uint64_t IST3;
    uint64_t IST4;
    uint64_t IST5;
    uint64_t IST6;
    uint64_t IST7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t IOPB_offset;
} __attribute__ ((packed)) tss_entry_t;

struct
{
    gdt_descriptor_t gdt_desc[5];
    tss_descriptor_t tss_desc[1];
} __attribute__ ((packed)) GDT __attribute__ ((aligned(8)));

gdtr_t gdtr;
tss_entry_t tss;

void gdt_init()
{
    memset(&GDT, 0, sizeof(GDT));

    // Kernel data descriptor
    GDT.gdt_desc[SELECTOR_KERNEL_DATA / 8].access = GDT_PRESENT | GDT_PRIV_KERNEL | GDT_TYPE | GDT_READ_WRITE;
    GDT.gdt_desc[SELECTOR_KERNEL_DATA / 8].limit_high_flags = GDT_64BIT;

    // Kernel code descriptor
    GDT.gdt_desc[SELECTOR_KERNEL_CODE / 8].access = GDT_PRESENT | GDT_PRIV_KERNEL | GDT_TYPE | GDT_EXEC | GDT_READ_WRITE;
    GDT.gdt_desc[SELECTOR_KERNEL_CODE / 8].limit_high_flags = GDT_64BIT;

    // User data descriptor
    GDT.gdt_desc[SELECTOR_USER_DATA / 8].access = GDT_PRESENT | GDT_PRIV_USER | GDT_TYPE | GDT_READ_WRITE;
    GDT.gdt_desc[SELECTOR_USER_DATA / 8].limit_high_flags = GDT_64BIT;

    // User code descriptor
    GDT.gdt_desc[SELECTOR_USER_CODE / 8].access = GDT_PRESENT | GDT_PRIV_USER | GDT_TYPE | GDT_EXEC | GDT_READ_WRITE;
    GDT.gdt_desc[SELECTOR_USER_CODE / 8].limit_high_flags = GDT_64BIT;

    memset(&tss, 0, sizeof(tss_entry_t));
    tss.IOPB_offset = sizeof(tss_entry_t);

    uint64_t base = (uint64_t)&tss;
    GDT.tss_desc[0].limit_low = sizeof(tss_entry_t) - 1;
    GDT.tss_desc[0].base_low = base & 0xFFFF;
    GDT.tss_desc[0].base_middle = (base >> 16) & 0xFF;
    GDT.tss_desc[0].access = TSS_PRESENT | TSS_PRIV_KERNEL | TSS_FREE;
    GDT.tss_desc[0].base_high = (base >> 24) & 0xFF;
    GDT.tss_desc[0].base_highest = base >> 32;

    gdtr.size = sizeof(GDT) - 1;
    gdtr.offset = (uint64_t)&GDT;

    asm volatile("lgdt %[addr]" : : [addr]"m"(gdtr) : "memory");
    asm volatile("mov ax, 0x28 \n ltr ax");
}

void tss_set_kstack(void* stack_top)
{
    tss.RSP0 = (uint64_t)stack_top;
}

void* tss_get_kstack()
{
    return (void*)tss.RSP0;
}