#pragma once

#include <stdint.h>
#include <interrupt/interrupt.h>

#define PIT_OCW_COUNTER_0 0
#define PIT_OCW_COUNTER_1 0x40
#define PIT_OCW_COUNTER_2 0x80

#define PIT_OCW_MODE_TERMINALCOUNT 00
#define PIT_OCW_MODE_ONESHOT 0x2
#define PIT_OCW_MODE_RATEGEN 0x4
#define PIT_OCW_MODE_SQUAREWAVEGEN 0x6
#define PIT_OCW_MODE_SOFTWARETRIG 0x8
#define PIT_OCW_MODE_HARDWARETRIG 0xA

typedef void(*pit_callback_t)(const interrupt_context_t*);

void pit_initialize();
uint64_t pit_get_ticks();
void pit_reset_counter(uint32_t freq, uint8_t counter, uint8_t mode);
void pit_register_callback(pit_callback_t callback);
