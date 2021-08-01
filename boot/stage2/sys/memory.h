#pragma once

#include "../lib/stdint.h"
#include "../lib/stdbool.h"
#include "../lib/stddef.h"

typedef struct mmap_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} __attribute__ ((packed)) mmap_entry_t;

typedef enum memory_type
{
    MEM_USABLE = 1,
    MEM_RESERVED,
    MEM_ACPI_RECLAIMABLE,
    MEM_ACPI_NVS,
    MEM_BAD
} memory_type_t;

extern mmap_entry_t memory_map[];

bool memory_read_map();
size_t memory_num_entries();
size_t memory_size();
size_t memory_lower_size();


