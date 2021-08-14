#pragma once

#include <stdint.h>
#include <interrupt/interrupt.h>

#define LAPIC_REG_ICR0      0x300
#define LAPIC_REG_ICR1      0x310
#define LAPIC_REG_SPURIOUS  0x0F0
#define LAPIC_REG_EOI       0x0B0
#define LAPIC_REG_ID        0x020
#define LAPIC_REG_TPR       0x080
#define LAPIC_REG_TIMER_DIV 0x3E0
#define LAPIC_REG_LVT_TIMER 0x320
#define LAPIC_REG_TIMER_INITIAL 0x380

#define LAPIC_ISR_DONE() { lapic_write(LAPIC_REG_EOI, 0); }

typedef void(*lapic_callback_t)(const interrupt_context_t*);

void apic_init();
void apic_setup();

uint8_t lapic_get_id();
void lapic_write(uint16_t reg, uint32_t value);
uint32_t lapic_read(uint16_t reg);
void lapic_timer_callback(lapic_callback_t callback);
