#include <x86/cpu.h>

#define PIC_PORT_CMD_MASTER     0x20
#define PIC_PORT_CMD_SLAVE      0xA0
#define PIC_PORT_DATA_MASTER    0x21
#define PIC_PORT_DATA_SLAVE     0xA1

#define PIC_CMD_EOI 0x20

#define ISR_DONE() { outportb(PIC_PORT_CMD_MASTER, PIC_CMD_EOI); outportb(PIC_PORT_CMD_SLAVE, PIC_CMD_EOI); }

typedef struct interrupt_context
{
    registers_t regs;            ///< all general-purpose registers.
    uint64_t    error;           ///< exception error identifier.
    uint64_t    interrupt;       ///< interrupt vector number.
    uint64_t    retaddr;         ///< interrupt return address.
    uint64_t    cs;              ///< code segment.
    uint64_t    rflags;          ///< flags register.
    uint64_t    rsp;             ///< stack pointer.
    uint64_t    ss;              ///< stack segment.
} interrupt_context_t;

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
