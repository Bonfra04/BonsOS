#pragma once

#include <stdint.h>

/**
 * @brief prepares the pit for use as one shot mode
 * @param[in] millis the number of milliseconds to wait
 */
void pit_prepare_one_shot(uint64_t millis);

/**
 * @brief performs the one shot prepared with pit_prepare_one_shot
 */
void pit_perform_one_shot();
