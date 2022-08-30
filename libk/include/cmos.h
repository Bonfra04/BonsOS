#pragma once

#include <stdint.h>

/**
 * @brief reads a byte value from cmos chip
 * @param[in] reg register address to read from
 * @return register value
 */
uint8_t cmos_read_byte(uint8_t reg);

/**
 * @brief writes a byte value to cmos chip
 * @param[in] reg register address to write to
 * @param[in] value value to write to the register
 */
void cmos_write_byte(uint8_t reg, uint8_t value);
