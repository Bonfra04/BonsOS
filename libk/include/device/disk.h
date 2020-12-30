#pragma once

#include <stdint.h>

typedef enum
{
    HARD_DISK = 0x80
} disk_type_t;

void read_disk(uint64_t address, uint32_t lba, uint16_t amount, disk_type_t diskType);