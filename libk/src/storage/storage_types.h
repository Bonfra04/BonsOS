#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <containers/vector.h>

#define MAX_BUFFER_LENGTH 512

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

typedef struct partition
{
    uint8_t type;
    size_t offset;
    size_t length;
} partition_t;

typedef struct storage_device
{
    size_t capacity;
    uint64_t internal_id;
    size_t lba_pos;

    uint8_t buffer[MAX_BUFFER_LENGTH];
    size_t buff_len;
    size_t buff_off;

    storage_function_t reader;
    storage_function_t writer;

    vector_t partitions;
    bool registered;
} storage_device_t;
