#include <pci/ata/ata.h>
#include <pci/pci.h>
#include <log.h>
#include <pci/ata/sata.h>

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

void ata_init()
{
    sata_init();
}

void ata_register_device(uint8_t bus, uint8_t dev, uint8_t fun)
{
    pci_device_t device = pci_get_device(bus, dev, fun);

    switch (device.subclass)
    {
    case PCI_ATA_SUB_SATA:
        pci_toggle_bus_master(bus, dev, fun, true);
        sata_register_device(&device);
        break;
    
    default:
        kernel_warn("Ignoring ATA device [subclass: %02X]", device.subclass);
        break;
    }
}
