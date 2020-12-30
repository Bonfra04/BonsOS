#include <device/ata/ide.h>
#include <panic.h>
#include <x86/ports.h>

#define MAX_IDES 4

#define ATA_DATA 0x0
#define ATA_ERROR 0x1
#define ATA_FEATURES 0x1
#define ATA_SECT_CNT 0x2
#define ATA_SECT_NUM 0x3
#define ATA_LBA_LOW 0x03
#define ATA_CYL_LOW 0x04
#define ATA_LBA_MID 0x4
#define ATA_CYL_HIGH 0x5
#define ATA_LBA_HIGH 0x5
#define ATA_DRV_HEAD 0x6
#define ATA_STATUS 0x7
#define ATA_COMMAND 0x7

#define ATA_ALT_STAT 0x2
#define ATA_DEV_CNTR 0x2
#define ATA_DEV_ADDR 0x3


static pci_device_t ides[MAX_IDES];
static size_t registered_devices;

void ide_init()
{
    registered_devices = 0;
}

void ide_register_device(pci_device_t* device)
{
    if(registered_devices >= MAX_IDES)
        kenrel_panic("Only %d IDE are supported.", MAX_IDES);

    if(device->base0 == 0) // device is in compatibility mode
    {
        // set BARs to de default values
        device->base0 = 0x1F0;
        device->base1 = 0x3F4;
        device->base2 = 0x170;
        device->base3 = 0x374;
    }
    else
    {
        if(device->base0 & 0x01) // device is attached to a port
            device->base0--;
        else
            return; // TODO: device is memory mapped and I don't know what I'm doing send help plz
    }

    ides[registered_devices++] = *device;
}