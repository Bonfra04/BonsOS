#pragma once

#include <drivers/storage/ata.h>
#include <drivers/pci.h>
#include <stddef.h>

/**
 * @brief initializes ahci
 */
void ahci_init();
