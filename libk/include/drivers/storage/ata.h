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

typedef struct ata_device
{
    void* data;
    bool (*send_ata_cmd)(void* device, ata_command_t* command, uint8_t* data, size_t transfer_len);
    
    bool lba48;
    size_t sector_size;
    size_t capacity;
} ata_device_t;

/**
 * @brief initializes ata
 */
void ata_init();

/**
 * @brief registers a ata devic
 * @param[in] ata_device ata device to register
 */
void ata_register_device(ata_device_t ata_device);
