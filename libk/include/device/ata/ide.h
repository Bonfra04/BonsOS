#pragma once

#include <device/pci.h>

void ide_init();

void ide_register_device(pci_device_t* device);