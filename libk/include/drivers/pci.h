#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct pci_device
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command_register;
    uint16_t status_register;
    uint8_t revision_id;
    uint8_t programming_interface;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t self_test;
    uint32_t base0;
    uint32_t base1;
    uint32_t base2;
    uint32_t base3;
    uint32_t base4;
    uint32_t base5;
    uint32_t cardbus;
    uint16_t subsys_vendor_id;
    uint16_t subsys_id;
    uint32_t rom_base;
    uint8_t capsoff;
    uint8_t resv0[3];
    uint8_t resv1[4];
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_time;
    uint8_t max_time;
    uint8_t varies[192];
} __attribute__ ((packed)) pci_device_t;

typedef struct pci_dev_info
{
    const pci_device_t dev;
    const uint8_t bus;
    const uint8_t device;
    const uint8_t function;
} pci_dev_info_t;

#define PCI_SUBCLASS_ANY 0xFF
#define PCI_PROGIF_ANY 0xFF

#define PCI_DRIVER_MATCH_NONE 0
#define PCI_DRIVER_MATCH_CLASS 1
#define PCI_DRIVER_MATCH_SUBCLASS 2
#define PCI_DRIVER_MATCH_PROGIF 4

typedef struct pci_driver
{
    uint8_t match;
    uint8_t class, subclass, progif;
    void (*register_device)(const pci_dev_info_t* device);
} pci_driver_t;

#define PCI_PRIV_PIO (1 << 0) 
#define PCI_PRIV_MMIO (1 << 1)
#define PCI_PRIV_DMA (1 << 2)

#define PCI_BAR_IO (1 << 0)

/**
 * @brief enumerates the pci bus initializing all connected devices
 */
void pci_init();

/**
 * @brief registers a driver for a specific pci class
 * @param[in] driver the driver to register
 */
void pci_register_driver(const pci_driver_t* driver);

/**
 * @brief sets privileges for the given device
 * @param device pointer to device
 * @param privileges privileges bitmask
 */
void pci_set_privileges(const pci_dev_info_t* device, uint8_t privileges);

/**
 * @brief fetch the pci returning the 256 bytes data structure describing the device
 * @param bus the bus number
 * @param device the device number
 * @param function the function number
 * @return device descriptor
 */
pci_dev_info_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function);

/**
 * @brief writes a byte from the given pci device inside the varies field and updates the varies field
 * @param[in] device pointer to the device
 * @param[in] offset offset from the start of the device descriptor (>= 0x40)
 * @param[in] value value to write
 */
void pci_write_byte(const pci_dev_info_t* device, uint8_t offset, uint8_t value);

/**
 * @brief writes a word from the given pci device inside the varies field and updates the varies field
 * @param[in] device pointer to the device
 * @param[in] offset offset from the start of the device descriptor (>= 0x40)
 * @param[in] value value to write
 */
void pci_write_word(const pci_dev_info_t* device, uint8_t offset, uint16_t value);

/**
 * @brief writes a dword from the given pci device inside the varies field and updates the varies field
 * @param[in] device pointer to the device
 * @param[in] offset offset from the start of the device descriptor (>= 0x40)
 * @param[in] value value to write
 */
void pci_write_dword(const pci_dev_info_t* device, uint8_t offset, uint32_t value);