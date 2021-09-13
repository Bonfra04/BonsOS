#pragma once

#include <device/pci.h>
#include <stdbool.h>

void ahci_init();

void ahci_register_pci_device(const pci_device_t* device);
size_t ahci_devices();
size_t ahci_get_capacity(size_t device);

bool sata_read(size_t device, uint64_t lba, uint8_t count, volatile void* address);
bool sata_write(size_t device, uint64_t lba, uint8_t count, void* address);