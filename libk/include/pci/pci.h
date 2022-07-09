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

/**
 * @brief enumerates the pci bus initializing all connected devices
 */
void pci_init();

/**
 * @brief sets the bus master bit for the given device
 * @param bus the bus number
 * @param device the device number
 * @param function the function number
 * @param enable true to enable, false to disable
 */ 
void pci_toggle_bus_master(uint8_t bus, uint8_t device, uint8_t function, bool enable);

/**
 * @brief fetch the pci returning the 256 bytes data structure describing the device
 * @param bus the bus number
 * @param device the device number
 * @param function the function number
 * @return device descriptor
 */
pci_device_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function);
