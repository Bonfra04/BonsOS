#pragma once

#include <device/pci.h>

void sata_init();

void sata_register_pci_device(pci_device_t* device);
size_t sata_devices();

void sata_read(size_t device, uint64_t lba, uint8_t count, void* address);
void sata_write(size_t device, uint64_t lba, uint8_t count, void* address);