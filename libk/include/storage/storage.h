#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define PART_TYPE_FREE  0x00
#define PART_TYPE_FAT16 0x06

typedef bool (*storage_function_t)(void* device, uint64_t lba, uint64_t num_sectors, void* address);

typedef struct storage_data
{
    size_t capacity;
    size_t sector_size;
    void* data;
    storage_function_t reader;
    storage_function_t writer;
} storage_data_t;

typedef struct storage_descriptor
{
    uint64_t device_id;
    size_t capacity;
    size_t partitions_count;
} storage_descriptor_t;

typedef struct partition_descriptor
{
    uint64_t device_id;
    size_t start;
    size_t length;
    uint8_t type;
} partition_descriptor_t;

/**
 * @brief initialize the abstraction of all storage devices and registers all already present devices
 */
void storage_init();

/**
 * @brief registers a storage device abstraction and returns its id
 * @param[in] data the storage device data
 * @return the id of the storage device
 */
uint64_t storage_register_device(storage_data_t data);

/**
 * @brief unregisters a storage device abstraction
 * @param[in] id the id of the device to unregister
 */
void storage_unregister_device(uint64_t id);

/**
 * @brief returns the number of registered storage devices
 * @return the number of registered storage devices
 */
size_t storage_ndevices();

/**
 * @brief returns a descriptor for the required device
 * @param[in] id the id of the device
 * @return the descriptor for the required device
 */
storage_descriptor_t storage_info(uint64_t id);

/**
 * @brief returns a descriptor for the required partition
 * @param[in] id the id of the device
 * @param[in] index the id of the partition
 * @return the descriptor for the required partition
 */
partition_descriptor_t storage_get_partition(uint64_t id, uint64_t index);

/**
 * @brief flushes the buffer of a storage device
 * @param[in] id the id of the device to flush
 * @return true if the device was found and flushed, false otherwise
 */
bool storage_flush(size_t id);

/**
 * @brief sets the cursor of a disk to a specific position
 * @param[in] id the id of the device to set the cursor
 * @param[in] position the position to set the cursor to in bytes
 * @return true if the device was found and the cursor was set, false otherwise
 */
bool storage_seek(size_t id, size_t position);

/**
 * @brief reads bytes from a storage device
 * @param[in] id the id of the device to read from
 * @param[in] amount the amount of bytes to read
 * @param[out] address the buffer to read the bytes into
 * @return number of bytes read
 */
uint64_t storage_read(size_t id, size_t amount, const void* address);

/**
 * @brief writes bytes to a storage device
 * @param[in] id the id of the device to write to
 * @param[in] amount the amount of bytes to write
 * @param[in] address the buffer to write the bytes from
 * @return number of bytes written
 */
uint64_t storage_write(size_t id, size_t amount, const void* address);

/**
 * @brief sets cursor to a specific position and reads bytes from a storage device
 * @param[in] id the id of the device to read from
 * @param[in] position the position to set the cursor to in bytes
 * @param[in] amount the amount of bytes to read
 * @param[out] address the buffer to read the bytes into
 * @return number of bytes read
 */
uint64_t storage_seek_read(size_t id, size_t position, size_t amount, const void* address);

/**
 * @brief sets cursor to a specific position and writes bytes to a storage device
 * @param[in] id the id of the device to write to
 * @param[in] position the position to set the cursor to in bytes
 * @param[in] amount the amount of bytes to write
 * @param[out] address the buffer to write the bytes from
 * @return number of bytes written
 */
uint64_t storage_seek_write(size_t id, size_t position, size_t amount, const void* address);
