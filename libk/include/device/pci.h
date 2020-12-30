#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_position_t;

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command_register;
    uint16_t status_register;
    uint8_t revision_id;
    uint8_t programming_interface;
    uint8_t sub_class;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t heder_type;
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

    pci_position_t position;
} pci_device_t;

uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t port);
uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t port);
uint32_t pci_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t port);

void pci_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint8_t value);
void pci_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint16_t value);
void pci_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t port, uint32_t value);

void pci_init();