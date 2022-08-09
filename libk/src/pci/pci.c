#include <pci/pci.h>
#include <io/ports.h>
#include <pci/ata/ata.h>
#include <log.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PCI_ADDR 0x0CF8
#define PCI_DATA 0x0CFC

#define PCI_MAX_BUS 256
#define PCI_MAX_DEVICE 32
#define PCI_MAX_FUNCTION 8

#define MULTI_FUNCTION (1 << 7)

#define PCI_HEADER_DEVICE  0x0
#define PCI_HEADER_BRIDGE  0x1
#define PCI_HEADER_CARDBUS 0x2

#define PCI_BUS_MASTER (1 << 2)

#define PCI_CLASS_ATA 0x01

static uint8_t pci_read_8(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return (inportd(PCI_DATA) >> (port & 3) * 8) & 0xFF;
}

static uint16_t pci_read_16(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    if(port & 3 > 2)
        return (pci_read_8(bus, device, function, port + 1) << 8) | pci_read_8(bus, device, function, port);

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return (inportd(PCI_DATA) >> (port & 3) * 8) & 0xFFFF;
}

static uint32_t pci_read_32(uint8_t bus, uint8_t device, uint8_t function, uint8_t port)
{
    if(port & 3 != 0)
        return (pci_read_16(bus, device, function, port + 2) << 16) | pci_read_16(bus, device, function, port);

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    return inportd(PCI_DATA);
}

static void pci_write_8(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint8_t value)
{
    int shift = (port & 3) * 8;
    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, (inportd(PCI_DATA) & ~(0xFF << shift)) | (value << shift));
}

static void pci_write_16(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint16_t value)
{
    if (port & 3 > 2)
    {
        pci_write_8(bus, device, function, port + 0, (uint8_t)(value & 0xFF));
        pci_write_8(bus, device, function, port + 1, (uint8_t)(value >> 8));
        return;
    }

    int shift = (port & 3) * 8;
    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, (inportd(PCI_DATA) & ~(0xFF << shift)) | (value << shift));
}

static void pci_write_32(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint32_t value)
{
    if (port & 3 != 0)
    {
        pci_write_16(bus, device, function, port + 0, (uint16_t)(value & 0xFFFF));
        pci_write_16(bus, device, function, port + 2, (uint16_t)(value >> 16));
        return;
    }

    outportd(PCI_ADDR, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (port & 0xFC));
    outportd(PCI_DATA, value);
}

static inline uint16_t pci_get_vendor(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_16(bus, device, function, offsetof(pci_device_t, vendor_id));
}

static inline uint16_t pci_get_command(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_16(bus, device, function, offsetof(pci_device_t, command_register));
}

static inline uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_8(bus, device, function, offsetof(pci_device_t, header_type));
}

static inline uint8_t pci_get_class(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_8(bus, device, function, offsetof(pci_device_t, class));
}

static inline void pci_set_command(uint8_t bus, uint8_t device, uint8_t function, uint16_t value)
{
    pci_write_16(bus, device, function, offsetof(pci_device_t, command_register), value);
}

static void register_device(uint8_t bus, uint8_t dev, uint8_t func)
{
    uint8_t class = pci_get_class(bus, dev, func);

    switch (class)
    {
    case PCI_CLASS_ATA:
        ata_register_device(bus, dev, func);
        break;

    default:
        kernel_warn("Ignoring PCI device [class: %02X]", class);
    }

    pci_get_device(bus, dev, func);
}

void pci_init()
{
    ata_init();

    for(int bus = 0; bus < PCI_MAX_BUS; bus++)
        for(int dev = 0; dev < PCI_MAX_DEVICE; dev++)
        {
            uint16_t vendor_id = pci_get_vendor(bus, dev, 0);
            if(vendor_id == UINT16_MAX)
                continue;

            uint8_t header_type = pci_get_header_type(bus, dev, 0);
            int functions = (header_type & MULTI_FUNCTION) ? PCI_MAX_FUNCTION : 1;

            for(int func = 0; func < functions; func++)
            {
                header_type = pci_get_header_type(bus, dev, func);
                header_type &= ~MULTI_FUNCTION;

                if(header_type != PCI_HEADER_DEVICE)
                    continue;

                register_device(bus, dev, func);
            }
        }
}

void pci_toggle_bus_master(uint8_t bus, uint8_t device, uint8_t function, bool enable)
{
    uint16_t command = pci_get_command(bus, device, function);
    if(enable)
        command |= PCI_BUS_MASTER;
    else
        command &= ~PCI_BUS_MASTER;
    pci_set_command(bus, device, function, command);
}

pci_device_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function)
{
    pci_device_t dev;
    uint32_t* dev_ptr = (uint32_t*)&dev;

    for(int i = 0; i < sizeof(pci_device_t) / sizeof(uint32_t); i++)
        dev_ptr[i] = pci_read_32(bus, device, function, i << 2);

    return dev;
}
