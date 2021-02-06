#include <device/pci.h>
#include <x86/ports.h>
#include <device/ata.h>
#include <stddef.h>
#include <stdlib.h>

#define PCI_ADDR 0x0CF8
#define PCI_DATA 0x0CFC

#define PCI_MAX_BUS 256
#define PCI_MAX_DEVICE 32
#define PCI_MAX_FUNCTION 8

#define PCI_CLASS_ATA 0x01

uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    outportb(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return (inportd(PCI_DATA) >> (port & 3) * 8) & 0xFF;
}

uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    if(port & 3 > 2)
        return (pci_read_byte(bus, device, function, port + 1) << 8) | pci_read_byte(bus, device, function, port);

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return (inportd(PCI_DATA) >> (port & 3) * 8) & 0xFFFF;
}

uint32_t pci_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    if(port & 3 != 0)
        return (pci_read_word(bus, device, function, port + 2) << 16) | pci_read_word(bus, device, function, port);

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return inportd(PCI_DATA);
}

void pci_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint8_t value)
{
    int shift = (port & 3) * 8;
    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, (inportd(PCI_DATA) & ~(0xFF << shift)) | (value << shift));
}

void pci_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint16_t value)
{
    if (port & 3 > 2)
    {
        pci_write_byte(bus, device, function, port + 0, (uint8_t)(value & 0xFF));
        pci_write_byte(bus, device, function, port + 1, (uint8_t)(value >> 8));
        return;
    }

    int shift = (port & 3) * 8;
    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, (inportd(PCI_DATA) & ~(0xFF << shift)) | (value << shift));
}

void pci_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint32_t value)
{
    if (port & 3 != 0)
    {
        pci_write_word(bus, device, function, port + 0, (uint16_t)(value & 0xFFFF));
        pci_write_word(bus, device, function, port + 2, (uint16_t)(value >> 16));
        return;
    }

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, value);
}

void pci_init()
{   
    ata_init();

    // register devices
    for(int bus = 0; bus < PCI_MAX_BUS; bus++)
        for(int dev = 0; dev < PCI_MAX_DEVICE; dev++)
            for(int func = 0; func < PCI_MAX_FUNCTION; func++)
                if(pci_read_word(bus, dev, func, 0) != UINT16_MAX)
                {
                    pci_device_t data;
                    uint32_t* dataptr = (uint32_t*)&data;

                    // read in the 256 bytes (64 dwords)
                    for(int i = 0; i < 64; i++)
                        dataptr[i] = pci_read_dword(bus, dev, func, i << 2);

                    data.position.bus = bus;
                    data.position.device = dev;
                    data.position.function = func;

                    switch (data.class)
                    {
                        case PCI_CLASS_ATA:
                            ata_register_device(&data);
                            break;
                    }
                }
}
