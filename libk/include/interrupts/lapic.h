#pragma once

#include <interrupts/isr_dispatcher.h>

#include <stdint.h>

typedef void(*lapic_callback_t)(const interrupt_context_t*);

/**
 * @brief initialize the needed structures for lapic management
 */
void lapic_init();

/**
 * @brief enable the lapic for the calling core
 */
void lapic_setup();

/**
 * @brief returns the calling core's lapic id
 */
uint8_t lapic_get_id();

/**
 * @brief sends an eoi to the lapic
 */
void lapic_eoi();

/**
 * @brief registers a callback for the calling core's lapic timer interrupt
 * @param[in] callback the callback to register
 */
void lapic_timer_callback(lapic_callback_t callback);

/**
 * @brief returns the boot core's lapic id
 * @return the boot core's lapic id
 */
uint8_t lapic_get_boot_id();
