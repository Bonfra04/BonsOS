#pragma once

#include <stdint.h>

#define FIRST_CLUSTER_INDEX_IN_FAT 3

#define VFAT_DIR_ENTRY          0x0F

typedef enum file_attribute
{
    READ_ONLY   = 0x01,
    HIDDEN      = 0x02,
    SYSTEM      = 0x04,
    VOLUME      = 0x08,
    SUBDIR      = 0x10,
    ARCHIVE     = 0x20,
    DEVICE      = 0x40,
    FREE        = 0x80,
} file_attribute_t;

typedef struct bios_parameter_block
{
    uint8_t OEM_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_entries;
    uint16_t total_sectors;
    uint8_t media;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads_per_cylinder;
    uint32_t hidden_sectors;
    uint32_t total_sectors_big;
} __attribute__((packed)) bios_parameter_block_t;

typedef struct bios_parameter_block_ext
{
    uint8_t drive_number;
    uint8_t unused;
    uint8_t ext_boot_signature;
    uint32_t serial_number;
    uint8_t volume_label[11];
    uint8_t file_system[8];
} __attribute__((packed)) bios_parameter_block_ext_t;

typedef struct bootsector
{
    uint8_t jump[3];
    bios_parameter_block_t bpb;
    bios_parameter_block_ext_t bpb_ext;
    uint8_t code[448];
    uint16_t signature;
} __attribute__((packed)) bootsector_t;

typedef struct mount_info
{
    uint32_t num_sectors;
    uint32_t fat_offset;
    uint32_t num_root_entries;
    uint32_t root_offset;
    uint32_t root_size_in_sectors;
    size_t root_size_in_bytes;
    uint32_t fat_size_in_sectors;
    size_t fat_size_in_bytes;
    uint32_t fat_entry_size;
    uint32_t bytes_per_sector;
    uint32_t bytes_per_cluster;
    uint32_t sectors_per_cluster;
    uint32_t sectors_per_fat;
    uint32_t first_cluster_sector;
    size_t data_sector_count;
    size_t data_cluster_count;
    size_t dir_entry_per_sector;
} mount_info_t;

typedef struct dir_entry
{
    union
    {
        uint8_t fullname[11];
        struct
        {
            uint8_t name[8];
            uint8_t ext[3];
        };
    };
    uint8_t flags;
    uint8_t reserved;
    uint8_t creation_time_ms; // tenths of a second
    uint16_t creation_time; // mul seconds by 2. hours: 5 bits; minutes: 6 bits; seconds: 5 bits;
    uint16_t creation_date; // year: 7 bits; month: 4 bits; day: 5 bits;
    uint16_t last_access_date;
    uint16_t unused;
    uint16_t last_edit_time;
    uint16_t last_edit_date;
    uint16_t first_cluster;
    uint32_t file_size;
} __attribute__((packed)) dir_entry_t;

typedef struct file_data
{
    size_t dir_entry_address;
    size_t first_cluster;
    size_t cluster;
    size_t offset;
    size_t bytes_left;
} file_data_t;
