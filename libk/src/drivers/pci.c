#include <drivers/pci.h>
#include <io/ports.h>
#include <log.h>

#include <containers/darray.h>

#include <assert.h>
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

static pci_dev_info_t* devices;
static pci_driver_t** drivers;

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

static inline uint16_t get_vendor(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_16(bus, device, function, offsetof(pci_device_t, vendor_id));
}

static inline uint8_t get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_8(bus, device, function, offsetof(pci_device_t, header_type));
}

static inline uint8_t get_privileges(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t command = pci_read_16(bus, device, function, offsetof(pci_device_t, command_register));
    return command & 0b111;
}

static inline void set_privileges(uint8_t bus, uint8_t device, uint8_t function, uint8_t privileges)
{
    uint16_t command = pci_read_8(bus, device, function, offsetof(pci_device_t, command_register));
    command &= ~0b111;
    command |= privileges;
    pci_write_16(bus, device, function, offsetof(pci_device_t, command_register), command);
}

void pci_init()
{
    devices = darray(pci_dev_info_t, 0);
    drivers = darray(pci_driver_t*, 0);

    for(int bus = 0; bus < PCI_MAX_BUS; bus++)
        for(int dev = 0; dev < PCI_MAX_DEVICE; dev++)
        {
            uint16_t vendor_id = get_vendor(bus, dev, 0);
            if(vendor_id == UINT16_MAX)
                continue;

            uint8_t header_type = get_header_type(bus, dev, 0);
            int functions = (header_type & MULTI_FUNCTION) ? PCI_MAX_FUNCTION : 1;

            for(int func = 0; func < functions; func++)
            {
                header_type = get_header_type(bus, dev, func);
                header_type &= ~MULTI_FUNCTION;

                if(header_type != PCI_HEADER_DEVICE)
                    continue;

                pci_dev_info_t device = pci_get_device(bus, dev, func);
                darray_append(devices, device);
            }
        }
}

void pci_register_driver(const pci_driver_t* driver)
{
    darray_append(drivers, driver);

    for(int i = 0; i < darray_length(devices); i++)
    {
        const pci_dev_info_t* device = &devices[i];

        if(driver->match & PCI_DRIVER_MATCH_CLASS && device->dev.class != driver->class)
            continue;
        if(driver->match & PCI_DRIVER_MATCH_SUBCLASS && device->dev.subclass != driver->subclass)
            continue;
        if(driver->match & PCI_DRIVER_MATCH_PROGIF && device->dev.programming_interface != driver->progif)
            continue;

        driver->register_device(device);
    }
}

void pci_set_privileges(const pci_dev_info_t* device, uint8_t privileges)
{
    uint16_t privs = get_privileges(device->bus, device->device, device->function);
    privs |= privileges;
    set_privileges(device->bus, device->device, device->function, privs);

    pci_device_t* dev = (pci_device_t*)&device->dev;
    dev->command_register &= 0b111;
    dev->command_register |= privileges;
}

pci_dev_info_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function)
{
    pci_device_t dev;
    uint32_t* dev_ptr = (uint32_t*)&dev;

    for(int i = 0; i < sizeof(pci_device_t) / sizeof(uint32_t); i++)
        dev_ptr[i] = pci_read_32(bus, device, function, i << 2);

    return (pci_dev_info_t){ .dev = dev, .bus = bus, .device = device, .function = function };
}

void pci_write_byte(const pci_dev_info_t* device, uint8_t offset, uint8_t value)
{
    pci_write_8(device->bus, device->device, device->function, offset, value);
    pci_device_t* dev = (pci_device_t*)&device->dev;
    ((uint8_t*)dev)[offset] = value;
}

void pci_write_word(const pci_dev_info_t* device, uint8_t offset, uint16_t value)
{
    assert(offset % 2 == 0);
    assert(offset >= 0x40 && offset < sizeof(pci_device_t) - 1);
    pci_write_16(device->bus, device->device, device->function, offset, value);
    pci_device_t* dev = (pci_device_t*)&device->dev;
    ((uint16_t*)dev)[offset / 2] = value;
}

void pci_write_dword(const pci_dev_info_t* device, uint8_t offset, uint32_t value)
{
    assert(offset % 4 == 0);
    assert(offset >= 0x40 && offset < sizeof(pci_device_t) - 3);
    pci_write_32(device->bus, device->device, device->function, offset, value);
    pci_device_t* dev = (pci_device_t*)&device->dev;
    ((uint32_t*)dev)[offset / 4] = value;
}