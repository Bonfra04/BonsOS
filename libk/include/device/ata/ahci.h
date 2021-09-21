#pragma once

#include <device/pci.h>
#include <stdbool.h>

void ahci_init();

void ahci_register_pci_device(const pci_device_t* device);
size_t ahci_devices();
size_t ahci_get_capacity(size_t device);

bool ahci_read(size_t device, uint64_t lba, uint8_t count, void* address);
bool ahci_write(size_t device, uint64_t lba, uint8_t count, void* address);