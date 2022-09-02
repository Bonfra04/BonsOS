#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ATA_CMD_READDMA     0xC8
#define ATA_CMD_READEXTDMA  0x25

#define ATA_CMD_WRITEDMA    0xCA
#define ATA_CMD_WRITEEXTDMA 0x35

#define ATA_CMD_IDENTIFY    0xEC

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
    uint64_t internal_id;
    bool (*send_ata_cmd)(uint64_t dev_id, ata_command_t* command, uint8_t* data, size_t transfer_len);
    
    bool lba48;
    size_t sector_size;
    size_t capacity;
} ata_device_t;

/**
 * @brief initializes ata
 */
void ata_init();

/**
 * @brief registers a ata device given pci address
 * @param[in] bus the bus number
 * @param[in] device the device number
 * @param[in] function the function number
 */
void ata_register_device(uint8_t bus, uint8_t dev, uint8_t fun);

/**
 * @brief return number of registered devices
 * @return number of registered devices
 */
size_t ata_ndevices();

/**
 * @brief sends an ata command to a ahci device
 * @param[in] dev_id device id to send command to
 * @param[in] command command to send
 * @param[inout] data transfer buffer
 * @param[in] transfer_len length of transfer buffer
 * @return success
 */ 
bool ata_send_cmd(uint64_t dev_id, ata_command_t* command, uint8_t* data, size_t transfer_len);

/**
 * @brief reads a block of data from an ata device
 * @param[in] dev_id device it to issue the read to
 * @param[in] lba the logical block address
 * @param[in] sectors the number of sectors to read
 * @param[out] buffer the buffer to read into
 * @return true on success, false on failure
 */
bool ata_read(uint64_t dev_id, uint64_t lba, size_t nsectors, void* buffer);

/**
 * @brief writes a block of data from an ata device
 * @param[in] dev_id device it to issue the write to
 * @param[in] lba the logical block address
 * @param[in] sectors the number of sectors to write
 * @param[out] buffer the buffer to write from
 * @return true on success, false on failure
 */
bool ata_write(uint64_t dev_id, uint64_t lba, size_t nsectors, void* buffer);


/**
 * @brief returns the size of a sector in bytes
 * @param[in] dev_id id of the device
 * @return the size of a sector in bytes
 */
size_t ata_sector_size(uint64_t dev_id);

/**
 * @brief returns the capacity of an ata device in bytes
 * @param[in] dev_id id of the device
 * @return the capacity of the device
 */
size_t ata_capacity(uint64_t dev_id);