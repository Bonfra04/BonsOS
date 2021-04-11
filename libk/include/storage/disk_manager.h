#pragma once

#include <filesystem/fsys.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*disk_function)(size_t device, uint64_t lba, uint8_t count, void* address); 

typedef struct partition
{
    uint8_t type;
    size_t lba_start;
    size_t byte_start;
    size_t length;
    file_system_t file_system;
} partition_t;

typedef struct disk
{
    partition_t partitions[4];
    size_t length;
    size_t position;
    uint8_t buffer[512];
    uint16_t offset;
    size_t device;
    disk_function reader;
    disk_function writer;
} disk_t;

/**
 * @brief initialize the disk manager
 */ 
void disk_manager_init();

/**
 * @brief Sets the cursor of a disk to a specific position.
 * @param disk_id ID of the disk to operate on.
 * @param position Address in byte of where to seek.
 * @return Seek success.
 */ 
bool disk_manager_seek(size_t disk_id, size_t position);

/**
 * @brief Reads bytes from a disk.
 * @param disk_id ID of the disk to operate on.
 * @param amount Amount of bytes to read.
 * @param buffer Address where the byets will be copied.
 * @return Amount of bytes read.
 */ 
size_t disk_manager_read(size_t disk_id, size_t amount, void* buffer);

/**
 * @brief Writes bytes to a disk.
 * @param disk_id ID of the disk to operate on.
 * @param amount Amount of bytes to write.
 * @param buffer Address from where the byets are copied.
 * @return Amount of bytes written.
 */ 
size_t disk_manager_write(size_t disk_id, size_t amount, void* buffer);

/**
 * @brief Flushes the buffer of the disk.
 * @param disk_id ID of the disk to operate on.
 * @return Flush success.
 */
bool disk_manager_flush(size_t disk_id);

#ifdef __cplusplus
}
#endif