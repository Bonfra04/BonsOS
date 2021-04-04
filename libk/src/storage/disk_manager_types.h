#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_PARTITIONS 26
#define MAX_DISKS 32

#define PTYPE_FREE  0x00
#define PTYPE_FAT16 0x06

typedef struct partition_entry
{
    uint8_t attributes;
    uint8_t start_h;
    uint8_t start_s;
    uint8_t start_c;
    uint8_t type;
    uint8_t end_h;
    uint8_t end_s;
    uint8_t end_c;
    uint32_t start_lba;
    uint32_t sectors;
} __attribute__((packed)) partition_entry_t;

typedef struct master_bootrecord
{
    uint8_t code[440];
    uint32_t disk_id;
    uint8_t reserved[2];
    partition_entry_t partitions[4];
    uint16_t signature;
} __attribute__((packed)) master_bootrecord_t;