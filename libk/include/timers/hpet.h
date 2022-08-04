#pragma once

#include <stdint.h>

/**
 * @brief initializes the hpet timer
 */
void hpet_init();

/**
 * @brief returns the amount of nanoseconds passed since boot
 * @return the amount of nanoseconds passed since boot
 */
uint64_t hpet_current_nanos();
