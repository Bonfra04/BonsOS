#pragma once

#include <stdint.h>
#include <stdbool.h>

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

typedef enum entry_type
{
    FAT16_FILE, FAT16_DIR, FAT16_ERROR_ENTRY
} entry_type_t;

typedef struct fat16_entry
{
    entry_type_t type;
    bool error;
    uint64_t first_cluster;
    uint64_t length;
    uint64_t advance;
    uint64_t cluster;
    uint64_t bytes_read;
} fat16_entry_t;

typedef struct lfn_entry
{
    uint8_t order;
    uint16_t name0[5];
    uint8_t lfn_attr;
    uint8_t zero0;
    uint8_t checksum;
    uint16_t name1[6];
    uint16_t zero1;
    uint16_t name2[2];
} __attribute__((packed)) lfn_entry_t;

#define CHARS_PER_LFN 13

#define INVALID_ENTRY (fat16_entry_t){ .error = true, .type = FAT16_ERROR_ENTRY }

#define FIRST_CLUSTER_INDEX_IN_FAT 3

typedef struct fat16_data
{
    uint64_t offset;
    fat16_entry_t root_dir;
    uint64_t storage_id;
    uint64_t bytes_per_cluster;
    uint64_t first_data_sector;
    uint64_t fat_offset;
} fat16_data_t;

typedef struct dir_entry
{
    union
    {
        char fullname[11];
        struct
        {
            char name[8];
            char ext[3];
        };
    };
    uint8_t flags;
    uint8_t reserved;
    uint8_t creation_time_ms;   // tenths of a second
    uint16_t creation_time;     // mul seconds by 2. hours: 5 bits; minutes: 6 bits; seconds: 5 bits;
    uint16_t creation_date;     // year: 7 bits; month: 4 bits; day: 5 bits;
    uint16_t last_access_date;
    uint16_t unused;
    uint16_t last_edit_time;
    uint16_t last_edit_date;
    uint16_t first_cluster;
    uint32_t file_size;
} __attribute__((packed)) dir_entry_t;

// utils
void from_dos(char dos[8+3], char* name);
file_t convert_entry(fat16_entry_t entry);
fat16_entry_t* convert_file(file_t* file);
bool get_next_cluster(fat16_data_t* data, uint64_t current_cluster, uint64_t* next_cluster);
void direntry_to_fatentry(dir_entry_t* d, fat16_entry_t* entry);

// read
bool internal_list_dir(fat16_data_t* data, fat16_entry_t* dir_entry, direntry_t* dirent);
fat16_entry_t get_entry(fat16_data_t* data, fat16_entry_t* directory, const char* filename);
size_t read_entry(fat16_data_t* data, fat16_entry_t* entry, void* buffer, size_t length);
bool read_dir(fat16_data_t* data, fat16_entry_t* dir, dir_entry_t* entry);

// write
fat16_entry_t create_entry(fat16_data_t* data, fat16_entry_t* dir, const char* filename);
size_t write_entry(fat16_data_t* data, fat16_entry_t* entry, void* buffer, size_t length);
bool remove_entry(fat16_data_t* data, fat16_entry_t* entry);
bool populate_file_entry(fat16_data_t* data, fat16_entry_t* entry);
bool populate_dir_entry(fat16_data_t* data, fat16_entry_t* entry);

#define ENTRY_FILE      0x00
#define ENTRY_READ_ONLY 0x01
#define ENTRY_HIDDEN    0x02
#define ENTRY_SYSTEM    0x04
#define ENTRY_VOLUME_ID 0x08
#define ENTRY_DIRECTORY 0x10
#define ENTRY_ARCHIVE   0x20
#define ENTRY_LFN       0x0F
