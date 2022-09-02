#pragma once

#include <pci/ata/ata.h>
#include <pci/pci.h>
#include <stddef.h>

/**
 * @brief initializes ahci
 */
void ahci_init();

/**
 * @brief registers an ahci device
 * @param[in] pci_device the pci device descriptor
 * @param[in] registrant function to call to register an ata device
 */
void ahci_register_device(pci_device_t* pci_device, void (*registrant)(ata_device_t*));

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
