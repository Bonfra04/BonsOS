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
    uint64_t base_address;
    uint64_t region_length;
    uint32_t region_type;
    uint32_t extended_attributes;
} memory_map_entry_t;

typedef struct memory_map
{
    memory_map_entry_t* entries;
    uint32_t num_entries;
    uint64_t total_memory;
} memory_map_t;

extern memory_map_t memory_map;

void memory_map_init(uint32_t num_entries, void* map_address, uint64_t total_memory);