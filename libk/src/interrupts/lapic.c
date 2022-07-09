#include <interrupts/lapic.h>
#include <cpu.h>
#include <memory/paging.h>
#include <interrupts/isr_dispatcher.h>
#include <pit.h>

#include <stdbool.h>
#include <string.h>

#define IA32_APIC_BASE_MSR 0x1B
#define LAPIC_MEM_SIZE 0x3F4

#define TIMER_ISR 0x20

#define LAPIC_REG_ICR0      0x300
#define LAPIC_REG_ICR1      0x310
#define LAPIC_REG_SPURIOUS  0x0F0
#define LAPIC_REG_EOI       0x0B0
#define LAPIC_REG_ID        0x020
#define LAPIC_REG_TPR       0x080
#define LAPIC_REG_TIMER_DIV 0x3E0
#define LAPIC_REG_LVT_TIMER 0x320
#define LAPIC_TIMER_INITCNT 0x380
#define LAPIC_TIMER_CURRCNT 0x390

#define TIMER_MODE_PERIODIC (0b01 << 17)

#define TIMER_DIV_1   0b1011
#define TIMER_DIV_2   0b0000
#define TIMER_DIV_4   0b0001
#define TIMER_DIV_8   0b0010
#define TIMER_DIV_16  0b0011
#define TIMER_DIV_32  0b1000
#define TIMER_DIV_64  0b1001
#define TIMER_DIV_128 0b1010

#define LAPIC_ENABLED 0x100
#define SPURIOUS_ISR 0xFF

static lapic_callback_t callbacks[256];
static bool initialized = false;
static uint32_t ticks_in_10ms = -1;
static uint8_t boot_id;

static void lapic_timer_isr(const interrupt_context_t* context)
{
    lapic_callback_t callback = callbacks[lapic_get_id()];
    if (callback)
        callback(context);

    lapic_eoi();
}

static void lapic_write(uint16_t reg, uint32_t value)
{
    uint8_t* lapic_addr = (uint8_t*)(rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000);
    *(volatile uint32_t*)(lapic_addr + reg) = value;
}

static uint32_t lapic_read(uint16_t reg)
{
    uint8_t* lapic_addr = (uint8_t*)(rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000);
    return *(volatile uint32_t*)(lapic_addr + reg);
}

static void timer_calibrate()
{
    pit_prepare_one_shot(10);
    
    lapic_write(LAPIC_TIMER_INITCNT, UINT32_MAX);
    
    pit_perform_one_shot();

    ticks_in_10ms = UINT32_MAX - lapic_read(LAPIC_TIMER_CURRCNT);
}

static void timer_setup()
{
    lapic_write(LAPIC_REG_LVT_TIMER, TIMER_ISR | TIMER_MODE_PERIODIC);
    lapic_write(LAPIC_REG_TIMER_DIV, TIMER_DIV_16);
    
    if(ticks_in_10ms == -1)
        timer_calibrate();

    lapic_write(LAPIC_TIMER_INITCNT, ticks_in_10ms);
}

void lapic_init()
{
    void* lapic_addr = (void*)(rdmsr(IA32_APIC_BASE_MSR) & 0xFFFFF000);
    paging_map(NULL, lapic_addr, lapic_addr, LAPIC_MEM_SIZE, PAGE_PRIVILEGE_KERNEL);

    boot_id = lapic_read(LAPIC_REG_ID) >> 24;
    memset(callbacks, 0, sizeof(callbacks));

    isr_set(TIMER_ISR, lapic_timer_isr);
    initialized = true;
}

void lapic_setup()
{
    lapic_write(LAPIC_REG_SPURIOUS, LAPIC_ENABLED | SPURIOUS_ISR);

    lapic_eoi();

    timer_setup();
}

uint8_t lapic_get_id()
{
    return initialized ? (lapic_read(LAPIC_REG_ID) >> 24) : 0;
}

void lapic_eoi()
{
    lapic_write(LAPIC_REG_EOI, 0);
}

void lapic_timer_callback(lapic_callback_t callback)
{
    callbacks[lapic_get_id()] = callback;
}

uint8_t lapic_get_boot_id()
{
    return boot_id;
}
