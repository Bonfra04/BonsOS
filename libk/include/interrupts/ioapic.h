#pragma once

#include <stdint.h>

#define IRQ_OFFSET 0x20

/**
 * @brief initialize the ioapics according to the madt tables
 */
void ioapic_init();

/**
 * @brief unmasks the given irq
 * @param[in] irq the irq to unmask
 */
void ioapic_unmask(uint8_t irq);

/**
 * @brief masks the given irq
 * @param[in] irq the irq to mask
 */
void ioapic_mask(uint8_t irq);

/**
 * @brief sends an eoi to the lapic of the calling core
 */
void ioapic_eoi();
