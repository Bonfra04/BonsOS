#include <interrupts/idt.h>
#include <linker.h>
#include <memory/gdt.h>

#include <stdint.h>

#define FLAG_PRESENT    (0x1 << 15)
#define FLAG_RING0      (0x0 << 13)
#define FLAG_RING3      (0x3 << 13)
#define FLAG_INTERRUPT  (0xE << 8)
#define FLAG_TRAP       (0xF << 11)

typedef struct idt_entry
{
    uint16_t offset_low;
    uint16_t segment;
    uint16_t flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__ ((packed)) idt_entry_t;

typedef struct idtr
{
    uint16_t limit;
    uint64_t offset;
} __attribute__ ((packed)) idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

extern symbol_t thunks;

void idt_init()
{
    for(uint16_t i = 0; i < 256; i++)
    {
        uint64_t addr = (uint64_t)&thunks + (i * 16);
        idt[i].offset_low = addr & 0xFFFF;
        idt[i].offset_mid = (addr >> 16) & 0xFFFF;
        idt[i].offset_high = (addr >> 32) & 0xFFFFFFFF;
        idt[i].segment = SELECTOR_KERNEL_CODE;
        idt[i].reserved = 0;
        idt[i].flags = (FLAG_PRESENT | FLAG_RING0 | FLAG_INTERRUPT);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.offset = (uint64_t)&idt;
}

void idt_install()
{
    asm("lidt %0" : : "m" (idtr));
}
