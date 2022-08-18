#pragma once

#include <stdint.h>

/**
 * @brief input 1 byte from port
 * @param port port number
 * @return byte read from port
 */
uint8_t inportb(uint16_t port);

/**
 * @brief output 1 byte to port
 * @param port port number
 * @param value byte to write to port
 */
void outportb(uint16_t port, uint8_t value);

/**
 * @brief input 2 bytes from port
 * @param port port number
 * @return 2 bytes read from port
 */
uint16_t inportw(uint16_t port);

/**
 * @brief output 2 bytes to port
 * @param port port number
 * @param value 2 bytes to write to port
 */
void outportw(uint16_t port, uint16_t value);

/**
 * @brief input 4 bytes from port
 * @param port port number
 * @return 4 bytes read from port
 */
uint32_t inportd(uint16_t port);

/**
 * @brief output 4 bytes to port
 * @param port port number
 * @param value 4 bytes to write to port
 */
void outportd(uint16_t port, uint32_t value);

/**
 * @brief waste one port cycle
 */
void port_wait();

/**
 * @brief waste n port cycles
 * @param cycles number of port cycles to waste
 */
void port_delay(uint64_t cycles);
