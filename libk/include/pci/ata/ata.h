#pragma once

#include <stdint.h>

/**
 * @brief initializes ata
 */
void ata_init();

/**
 * @brief registers a ata device given pci address
 * @param[in] bus the bus number
 * @param[in] device the device number
 * @param[in] function the function number
 */
void ata_register_device(uint8_t bus, uint8_t dev, uint8_t fun);
