#pragma once

#include <stdint.h>
#include <x86/ports.h>
#include <device/pci.h>

#define PCI_CLASS_ATA 0x01
#define PCI_ATA_SUB_SCSI 0x00
#define PCI_ATA_SUB_IDE 0x01
#define PCI_ATA_SUB_FDC 0x02
#define PCI_ATA_SUB_IPI 0x03
#define PCI_ATA_SUB_RAID 0x04
#define PCI_ATA_SUB_ATA 0x05
#define PCI_ATA_SUB_SATA 0x06
#define PCI_ATA_SUB_SAS 0x07
#define PCI_ATA_SUB_SSS 0x08
#define PCI_ATA_SUB_OTHER 0x80

void ata_init();

void ata_register_device(pci_device_t* device);