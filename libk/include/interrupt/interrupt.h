#pragma once

#include <x86/cpu.h>
#include <x86/ports.h>

#define PIC_CMD_MASTER  0x20
#define PIC_CMD_SLAVE   0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE  0xA1
#define PIC_CMD_EOI     0x20

#define ISR_DONE() { outportb(PIC_CMD_SLAVE, PIC_CMD_EOI); outportb(PIC_CMD_MASTER, PIC_CMD_EOI); }

typedef struct interrupt_context
{
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    registers_t regs;
    uint64_t error;
    uint64_t interrupt;
    uint64_t retaddr;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__ ((packed)) interrupt_context_t;

typedef void (*isr_handler_t)(const interrupt_context_t *context);

#ifdef __cplusplus
extern "C" {
#endif

void interrupts_init();

void isr_set(int interrupt, isr_handler_t handler);
void irq_enable(uint8_t irq);
void irq_disable(uint8_t irq);

#ifdef __cplusplus
}
#endif
