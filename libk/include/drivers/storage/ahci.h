#pragma once

#include <drivers/storage/ata.h>
#include <drivers/pci.h>
#include <stddef.h>

/**
 * @brief initializes ahci
 */
void ahci_init();

/**
 * @brief return number of registered devices
 * @return number of registered devices
 */
size_t ahci_ndevices();

/**
 * @brief sends an ata command to a ahci device
 * @param[in] dev_id device id to send command to
 * @param[in] command command to send
 * @param[inout] data transfer buffer
 * @param[in] transfer_len length of transfer buffer
 * @return success
 */ 
bool ahci_send_ata_cmd(uint64_t dev_id, ata_command_t* command, uint8_t* data, size_t transfer_len);
