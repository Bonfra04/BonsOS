#include <interrupt/interrupt.h>
#include <stdint.h>

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

// Special interrupts
#define EX_NMI 0x02
#define EX_DF 0x08
#define EX_TS 0x0A
#define EX_NP 0x0B
#define EX_SS 0x0C
#define EX_GP 0x0D
#define EX_PF 0x0E
#define EX_MC 0x12

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

typedef struct thunk
{
    uint8_t nop;
    uint8_t push;
    uint8_t interrupt;
    uint8_t jmp;
    uint32_t dispatcher;
} __attribute__ ((packed)) thunk_t;

static thunk_t thunks[256];
static idt_entry_t idt[256];
static idtr_t idtr;
isr_handler_t isr_handlers[256];

extern void isr_dispatcher();
extern void isr_dispatcher_special();

static void remap_pic()
{
    // int PICs
    outportb(PIC_CMD_MASTER, ICW1_INIT | ICW1_ICW4);
    outportb(PIC_CMD_SLAVE, ICW1_INIT | ICW1_ICW4);

    // set PICs offsets
    outportb(PIC_DATA_MASTER, 0x20);
    outportb(PIC_DATA_SLAVE, 0x28);

    // set PICs correspondings
    outportb(PIC_DATA_MASTER, 4);
    outportb(PIC_DATA_SLAVE, 2);

    // set PICs in 8086 mode
    outportb(PIC_DATA_MASTER, ICW4_8086);
    outportb(PIC_DATA_SLAVE, ICW4_8086);
    
    // diable all ISRs
    outportb(PIC_DATA_MASTER, 0xFF);
    outportb(PIC_DATA_SLAVE, 0xFF);
}

void interrupts_init()
{
    remap_pic();

    for(int i = 0; i < 256; i++)
    {
        // initialize thunk
        thunks[i].nop = 0x90;
        thunks[i].push = 0x6A;
        thunks[i].interrupt = i;
        thunks[i].jmp = 0xE9;

        thunks[i].dispatcher = (uint32_t)(uint64_t)isr_dispatcher;

        if(i <= 14)
        {
            int mask = (1 << i);
            int pushesError = (1 << EX_DF) | (1 << EX_TS) | (1 << EX_NP) | (1 << EX_SS) | (1 << EX_GP) | (1 << EX_PF);
            if(mask & pushesError != 0)
                thunks[i].dispatcher = (uint32_t)(uint64_t)isr_dispatcher_special;
        }            

        thunks[i].dispatcher -= (uint32_t)(uint64_t)&(thunks[i]) + sizeof(thunk_t);

        // install descriptor
        uint64_t addr = (uint64_t)&(thunks[i]);
        idt[i].offset_low = (uint16_t)(addr & 0x000000000000FFFF);
        idt[i].offset_mid = (uint16_t)((addr & 0x00000000FFFF0000) >> 16);
        idt[i].offset_high = (uint32_t)((addr & 0xFFFFFFFF00000000) >> 32);
        idt[i].segment = 0x10;
        idt[i].reserved = 0;

        if(i == EX_NMI)     // NMI exception (IST=1, Type=interrupt, DPL=0, P=1)
            idt[i].flags = 0b1000111000000001;
        else if(i == EX_DF) // Double-fault exception (IST=2, Type=interrupt, DPL=0, P=1)
            idt[i].flags = 0b1000111000000010;
        else if(i == EX_MC) // Machine-check exception (IST=3, Type=interrupt, DPL=0, P=1)
            idt[i].flags = 0b1000111000000011;
        if(i < 32)          // store the flags (IST=0, Type=interrupt, DPL=0, P=1)
            idt[i].flags = 0b1000111000000000;
        else                // store the flags (IST=0, Type=trap, DPL=0, P=1)            
            idt[i].flags = 0b1000111100000000;
    }

    // load idt
    idtr.limit = sizeof(idt);
    idtr.offset = (uint64_t)&idt;
    asm ("lidt %0" : : "m" (idtr));
}

void irq_enable(uint8_t irq)
{
    if(irq <= 8) // master
    {
        uint8_t bit = ~(1 << irq);
        uint8_t mask = inportb(PIC_DATA_MASTER);
        mask &= bit;
        outportb(PIC_DATA_MASTER, mask);
    }
    else // slave
    {
        irq_enable(2);
        irq -= 8;
        uint8_t bit = ~(1 << irq);
        uint8_t mask = inportb(PIC_DATA_SLAVE);
        mask &= bit;
        outportb(PIC_DATA_SLAVE, mask);
    }
}

void irq_disable(uint8_t irq)
{
    if(irq <= 8) // master
    {
        uint8_t bit = 1 << irq;
        uint8_t mask = inportb(PIC_DATA_MASTER);
        mask |= bit;
        outportb(PIC_DATA_MASTER, mask);
    }
    else // slave
    {
        irq -= 8;
        uint8_t bit = 1 << irq;
        uint8_t mask = inportb(PIC_DATA_SLAVE);
        mask |= bit;
        outportb(PIC_DATA_SLAVE, mask);
    }
}

void isr_set(int interrupt, isr_handler_t handler)
{
    isr_handlers[interrupt] = handler;
}