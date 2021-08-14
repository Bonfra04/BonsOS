#include <interrupt/apic.h>
#include <x86/cpu.h>
#include <panic.h>
#include <memory/paging.h>
#include <string.h>
#include <smp/atomic.h>

#define IA32_APIC_BASE_MSR 0x1B
#define LAPIC_MEM_SIZE 0x3F4

#define TIMER_DIV_1   0b1011
#define TIMER_DIV_2   0b0000
#define TIMER_DIV_4   0b0001
#define TIMER_DIV_8   0b0010
#define TIMER_DIV_16  0b0011
#define TIMER_DIV_32  0b1000
#define TIMER_DIV_64  0b1001
#define TIMER_DIV_128 0b1010

#define TIMER_MODE_PERIODIC (0b01 << 17)

#define TIMER_ISR 0x20
#define SPURIOUS_ISR 0xFF

static lapic_callback_t callbacks[256];

static void lapic_timer_isr(interrupt_context_t* context)
{
    uint8_t lapic_id = lapic_get_id();

    lapic_callback_t callback = locked_read(&callbacks[lapic_id]);
    if(callback)
        callback(context);

    LAPIC_ISR_DONE();
}

static void lapic_timer_init()
{
    // set and enable timer handler
    lapic_write(LAPIC_REG_LVT_TIMER, TIMER_ISR | TIMER_MODE_PERIODIC);

    // set timer divider
    lapic_write(LAPIC_REG_TIMER_DIV, TIMER_DIV_2);

    // set timer initial count
    lapic_write(LAPIC_REG_TIMER_INITIAL, 0xFFFFFF);
}

inline void lapic_write(uint16_t reg, uint32_t value)
{
    uint8_t* lapic_addr = (uint8_t*)(rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000);
    *(volatile uint32_t*)(lapic_addr + reg) = value;
}

inline uint32_t lapic_read(uint16_t reg)
{
    uint8_t* lapic_addr = (uint8_t*)(rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000);
    return locked_read((volatile uint32_t*)(lapic_addr + reg));
}

inline uint8_t lapic_get_id()
{
    return (uint8_t)(lapic_read(LAPIC_REG_ID) >> 24);
}

void apic_init()
{
    // Request the CPU's extended features.
    registers4_t regs;
    cpuid(1, &regs);
    if(!(regs.rdx & (1 << 9)))
        kenrel_panic("APIC not supported");

    void* lapic_addr = rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000;
    paging_map_global(lapic_addr, lapic_addr, LAPIC_MEM_SIZE, PAGE_PRIVILEGE_KERNEL);

    memset(callbacks, 0, sizeof(callbacks));

    // register timer handler
    isr_set(TIMER_ISR, lapic_timer_isr);
    apic_setup();
}

void apic_setup()
{
    // enable apic and set spurious interrupt vetor
    lapic_write(LAPIC_REG_SPURIOUS, 0x100 | 0xFF);

    // clear any spurious interrupt
    LAPIC_ISR_DONE();

    lapic_timer_init();
}

inline void lapic_timer_callback(lapic_callback_t callback)
{
    locked_write(&callbacks[lapic_get_id()], callback);
}
