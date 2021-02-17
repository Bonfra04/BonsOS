#include <device/pit.h>
#include <interrupt/interrupt.h>

#define PIT_REG_COUNTER0 0x40
#define PIT_REG_COUNTER1 0x41
#define PIT_REG_COUNTER2 0x42
#define PIT_REG_COMMAND 0x43

#define IRQ_PIT 0
#define TRAP_IRQ_PIT 0x20

#define PIT_OCW_MASK_BINCOUNT 1
#define PIT_OCW_MASK_MODE 0xE
#define PIT_OCW_MASK_RL 0x30
#define PIT_OCW_MASK_COUNTER 0xC0

#define PIT_OCW_BINCOUNT_BINARY 0
#define PIT_OCW_BINCOUNT_BCD 1

#define PIT_OCW_RL_LATCH 0
#define PIT_OCW_RL_LSBONLY 0x10
#define PIT_OCW_RL_MSBONLY 0x20
#define PIT_OCW_RL_DATA 0x30

static volatile uint64_t ticks;

static void isr_pit(const interrupt_context_t* context)
{
    (void)context;
    ticks++;
    ISR_DONE();
}

static void send_data(uint16_t data, uint8_t counter)
{
    uint8_t port =
        counter == PIT_OCW_COUNTER_0
        ? PIT_REG_COUNTER0
        : (
            counter == PIT_OCW_COUNTER_1
            ? PIT_REG_COUNTER1
            : PIT_REG_COUNTER2
        );

    outportb(port, (uint8_t)data);
}

void pit_reset_counter(uint32_t freq, uint8_t counter, uint8_t mode)
{
    if (freq == 0)
        return;

    uint16_t divisor = (uint16_t)(1193181 / freq);

    uint8_t ocw = 0;
    ocw = (ocw & ~PIT_OCW_MASK_MODE) | mode;
    ocw = (ocw & ~PIT_OCW_MASK_RL) | PIT_OCW_RL_DATA;
    ocw = (ocw & ~PIT_OCW_MASK_COUNTER) | counter;
    outportb(PIT_REG_COMMAND, ocw);

    send_data(divisor & 0xff, 0);
    send_data((divisor >> 8) & 0xff, 0);

    ticks = 0;
}

uint8_t i86_pit_read_data (uint16_t counter)
{
    uint8_t port =
        counter == PIT_OCW_COUNTER_0
        ? PIT_REG_COUNTER0
        : (
            counter == PIT_OCW_COUNTER_1
            ? PIT_REG_COUNTER1
            : PIT_REG_COUNTER2
        );

    return inportb(port);
}

uint64_t pit_get_ticks()
{
    return ticks;
}

void pit_initialize()
{
    isr_set(TRAP_IRQ_PIT, isr_pit);
    irq_enable(IRQ_PIT);

    ticks = 0;
}