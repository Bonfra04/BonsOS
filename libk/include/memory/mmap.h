#pragma once

#include <stdint.h>

typedef enum memory_type
{
    MEM_USABLE = 1,
    MEM_RESERVED,
    MEM_ACPI_RECLAIMABLE,
    MEM_ACPI_NVS,
    MEM_BAD
} memory_type_t;

typedef struct memory_map_entry
{
    void* base_address;
    uint64_t region_length;
    uint32_t region_type;
    uint32_t extended_attributes;
} __attribute__((packed)) mmap_entry_t;

typedef struct memory_map
{
    mmap_entry_t* entries;
    uint32_t num_entries;
    uint64_t total_memory;
} mmap_t;

/**
 * @brief initializes the memory map
 * @param[in] num_entries number of entries in the memory map
 * @param[in] map_address address of the memory map loaded by the BIOS
 * @param[in] total_memory size of total memory in bytes
 */
void mmap_init(uint32_t num_entries, void* map_address, uint64_t total_memory);

/**
 * @brief returns a pointer to the memory map
 * @return pointer to the memory map
 */
const mmap_t* mmap_get();
