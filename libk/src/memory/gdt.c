#include <memory/gdt.h>
#include <memory/heap.h>
#include <interrupts/lapic.h>

#include <linker.h>

#include <stdint.h>
#include <string.h>

#define GDT_PRESENT (1 << 7)
#define GDT_PRIV_KERNEL (0 << 5)
#define GDT_PRIV_USER (3 << 5)
#define GDT_TYPE (1 << 4)
#define GDT_EXEC (1 << 3)
#define GDT_READ_WRITE (1 << 1)

#define GDT_64BIT (1 << 5)

#define TSS_PRESENT (1 << 7)
#define TSS_PRIV_KERNEL (0 << 5)
#define TSS_PRIV_USER (3 << 5)
#define TSS_FREE (0b1001 << 0)
#define TSS_BUSY (0b1011 << 0)

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

typedef struct gdt
{
    gdt_descriptor_t gdt_desc[5];
    tss_descriptor_t tss_desc;
} __attribute__ ((packed)) __attribute__ ((aligned(8))) gdt_t;

typedef struct gdtr
{
    uint16_t size;
    uint64_t offset;
} __attribute__ ((packed)) gdtr_t;

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

static tss_entry_t boot_tss;
static gdt_t boot_gdt;
static gdtr_t boot_gdtr;

static gdt_t* gdt_create(uint8_t core_id)
{
    tss_entry_t* tss_entry = core_id == 0 ? &boot_tss : (tss_entry_t*)heap_malloc(sizeof(tss_entry_t));
    memset(tss_entry, 0, sizeof(tss_entry_t));
    tss_entry->IOPB_offset = sizeof(tss_entry_t);
    uint64_t tss_base = (uint64_t)tss_entry;

    gdt_t* gdt = core_id == 0 ? &boot_gdt : (gdt_t*)heap_malloc(sizeof(gdt_t));
    memset(gdt, 0, sizeof(gdt_t));

    // Kernel data descriptor
    gdt->gdt_desc[SELECTOR_KERNEL_DATA / 8].access = GDT_PRESENT | GDT_PRIV_KERNEL | GDT_TYPE | GDT_READ_WRITE;
    gdt->gdt_desc[SELECTOR_KERNEL_DATA / 8].limit_high_flags = GDT_64BIT;

    // Kernel code descriptor
    gdt->gdt_desc[SELECTOR_KERNEL_CODE / 8].access = GDT_PRESENT | GDT_PRIV_KERNEL | GDT_TYPE | GDT_EXEC | GDT_READ_WRITE;
    gdt->gdt_desc[SELECTOR_KERNEL_CODE / 8].limit_high_flags = GDT_64BIT;

    // User data descriptor
    gdt->gdt_desc[SELECTOR_USER_DATA / 8].access = GDT_PRESENT | GDT_PRIV_USER | GDT_TYPE | GDT_READ_WRITE;
    gdt->gdt_desc[SELECTOR_USER_DATA / 8].limit_high_flags = GDT_64BIT;

    // User code descriptor
    gdt->gdt_desc[SELECTOR_USER_CODE / 8].access = GDT_PRESENT | GDT_PRIV_USER | GDT_TYPE | GDT_EXEC | GDT_READ_WRITE;
    gdt->gdt_desc[SELECTOR_USER_CODE / 8].limit_high_flags = GDT_64BIT;

    // TSS descriptor
    gdt->tss_desc.base_low = tss_base & 0xFFFF;
    gdt->tss_desc.base_middle = (tss_base >> 16) & 0xFF;
    gdt->tss_desc.base_high = (tss_base >> 24) & 0xFF;
    gdt->tss_desc.base_highest = tss_base >> 32;
    gdt->tss_desc.limit_low = sizeof(tss_entry_t) - 1;
    gdt->tss_desc.access = TSS_PRESENT | TSS_PRIV_KERNEL | TSS_FREE;

    tss_entry->RSP0 = 0xBAADBEEF;

    return gdt;
}

void gdt_install()
{
    uint8_t core_id = lapic_get_id();

    gdt_t* gdt = gdt_create(0);
    
    gdtr_t* gdtr = core_id == 0 ? &boot_gdtr : (gdtr_t*)heap_malloc(sizeof(gdtr_t));
    gdtr->size = sizeof(gdt_t) - 1;
    gdtr->offset = (uint64_t)gdt;

    asm volatile("lgdt %[addr]" : : [addr]"m"(*gdtr) : "memory");
    asm volatile("ltr %[val]" : : [val]"r"((uint16_t)SELECTOR_TSS) : "memory");
}

void tss_set_kstack(void* stack_top)
{
    gdtr_t gdtr;
    asm volatile ("sgdt %0" : "=m"(gdtr));
    gdt_t* gdt = (gdt_t*)gdtr.offset;
    tss_descriptor_t* tss_desc = &gdt->tss_desc;
    tss_entry_t* tss_entry = (tss_entry_t*)((uint64_t)tss_desc->base_highest << 32 | tss_desc->base_high << 24 | tss_desc->base_middle << 16 | tss_desc->base_low);

    tss_entry->RSP0 = (uint64_t)stack_top;
}

void* tss_get_kstack()
{
    gdtr_t gdtr;
    asm volatile ("sgdt %0" : "=m"(gdtr));
    gdt_t* gdt = (gdt_t*)gdtr.offset;
    tss_descriptor_t* tss_desc = &gdt->tss_desc;
    tss_entry_t* tss_entry = (tss_entry_t*)((uint64_t)tss_desc->base_highest << 32 | tss_desc->base_high << 24 | tss_desc->base_middle << 16 | tss_desc->base_low);

    return ptr(tss_entry->RSP0);
}
