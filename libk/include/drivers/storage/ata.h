#pragma once

#include <drivers/pci.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct ata_command
{
    uint8_t command;
    uint16_t features;
    uint64_t lba;
    uint16_t n_sectors;

    bool write, lba28;
} ata_command_t;

typedef struct atapi_command
{
    uint16_t packet[16];
    
    bool write;
} atapi_command_t;

typedef struct ata_driver
{
    void* data;
    bool (*send_ata_cmd)(void* device, ata_command_t* command, uint8_t* data, size_t transfer_len);
    bool (*send_atapi_cmd)(void* device, atapi_command_t* command, uint8_t* data, size_t transfer_len);
    bool atapi;
} ata_driver_t;

/**
 * @brief initializes ata
 */
void ata_init();

/**
 * @brief registers a ata devic
 * @param[in] driver driver for the ata device
 */
void ata_register_device(ata_driver_t driver);
