#pragma once

#include <pci/pci.h>
#include <stddef.h>

/**
 * @brief initializes sata
 */
void sata_init();

/**
 * @brief registers a sata device
 * @param[in] device the device descriptor
 */
void sata_register_device(const pci_device_t* device);

/**
 * @brief returns the number of sata devices
 * @return the number of sata devices
 */
size_t sata_ndisks();

/**
 * @brief reads a block of data from a sata device
 * @param[in] disk the disk index
 * @param[in] lba the logical block address
 * @param[in] sectors the number of sectors to read
 * @param[out] buffer the buffer to read into
 * @return true on success, false on failure
 */
bool sata_read(size_t disk, uint64_t lba, size_t sectors, void* buffer);

/**
 * @brief writes a block of data to a sata device
 * @param[in] disk the disk index
 * @param[in] lba the logical block address
 * @param[in] sectors the number of sectors to write
 * @param[in] buffer the buffer to write from
 * @return true on success, false on failure
 */
bool sata_write(size_t disk, uint64_t lba, size_t sectors, void* buffer);

/**
 * @brief returns the capacity of a disk in bytes
 * @param[in] disk the disk index
 * @return the capacity of the disk in bytes
 */ 
size_t sata_get_capacity(size_t disk);

/**
 * @brief returns the size of a sector in bytes
 * @param[in] disk the disk index
 * @return the size of a sector in bytes
 */
size_t sata_get_sector_size(size_t disk);

/**
 * @brief gets the model of a disk to a string
 * @param[in] disk the disk index
 * @param[out] model the buffer to write the model to (at least 40 bytes)
 */
void sata_get_model(size_t disk, char* model);
