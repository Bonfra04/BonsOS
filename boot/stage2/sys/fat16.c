#include "fat16.h"
#include "disk.h"
#include "../lib/string.h"

#define VFAT_DIR_ENTRY          0x0F

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
    uint32_t root_size_in_bytes;
    uint32_t fat_size_in_sectors;
    uint32_t fat_size_in_bytes;
    uint32_t fat_entry_size;
    uint32_t bytes_per_sector;
    uint32_t bytes_per_cluster;
    uint32_t sectors_per_cluster;
    uint32_t sectors_per_fat;
    uint32_t first_cluster_sector;
    uint32_t data_sector_count;
    uint32_t data_cluster_count;
    uint32_t dir_entry_per_sector;
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

static uint32_t fs_offset;
static mount_info_t mount_info;

void fsys_init(uint32_t partition_offset)
{
    fs_offset = partition_offset;

    bootsector_t bootsector;
    disk_seek(fs_offset);
    disk_read(512, (void*)&bootsector);

    bios_parameter_block_t* bpb = &(bootsector.bpb);

    mount_info.num_sectors = bpb->total_sectors != 0 ? bpb->total_sectors : bpb->total_sectors_big;
    mount_info.bytes_per_sector = bpb->bytes_per_sector;
    mount_info.fat_offset = bpb->reserved_sectors * mount_info.bytes_per_sector;
    mount_info.fat_size_in_sectors = bpb->sectors_per_fat;
    mount_info.fat_size_in_bytes = mount_info.fat_size_in_sectors * bpb->bytes_per_sector;
    mount_info.fat_entry_size = 2;
    mount_info.num_root_entries = bpb->root_entries;
    mount_info.root_offset = ((bpb->number_of_fats * bpb->sectors_per_fat) + bpb->reserved_sectors) * mount_info.bytes_per_sector;
    mount_info.root_size_in_sectors = (bpb->root_entries * sizeof(dir_entry_t)) / bpb->bytes_per_sector;
    mount_info.root_size_in_bytes = mount_info.root_size_in_sectors * bpb->bytes_per_sector;
    mount_info.sectors_per_cluster = bpb->sectors_per_cluster;
    mount_info.sectors_per_fat = bpb->sectors_per_fat;
    mount_info.first_cluster_sector = mount_info.root_offset + mount_info.root_size_in_bytes;
    mount_info.bytes_per_cluster = bpb->bytes_per_sector * bpb->sectors_per_cluster;
    mount_info.data_sector_count = mount_info.num_sectors - (bpb->reserved_sectors + (bpb->number_of_fats * bpb->sectors_per_fat) + mount_info.root_size_in_sectors);
    mount_info.data_cluster_count = mount_info.data_sector_count / bpb->sectors_per_cluster;
    mount_info.dir_entry_per_sector = mount_info.bytes_per_sector / sizeof(dir_entry_t);
}

static void seek_to_root_region(uint32_t entry_index)
{
    uint32_t pos = fs_offset;
    pos += mount_info.root_offset;
    pos += entry_index * sizeof(dir_entry_t);
    disk_seek(pos);
}

static void seek_to_data_region(uint32_t cluster, uint16_t offset)
{
    uint32_t tmp = cluster - 2;
    tmp *= mount_info.bytes_per_cluster;

    uint32_t pos = fs_offset;
    pos += mount_info.first_cluster_sector;
    pos += tmp;
    pos += offset;
    disk_seek(pos);
}

static uint32_t find_entry(const char* entryname)
{
    seek_to_root_region(0);

    for(uint32_t i = 0; i < mount_info.num_root_entries; i++)
    {
        dir_entry_t entry;
        disk_read(sizeof(dir_entry_t), (void*)&entry);

        if(entry.flags & FREE)
            continue;

        if(!entry.fullname[0])
            break;

        // Ignore VFAT entries
        if(entry.flags & VFAT_DIR_ENTRY)
            continue;

        if(!memcmp(entryname, entry.fullname, sizeof(entry.fullname)))
            return i;
    }

    return -1;
}

static void seek_to_fat_region(uint32_t cluster)
{
    uint32_t pos = fs_offset;
    pos += mount_info.fat_offset;
    pos += cluster * 2;
    disk_seek(pos);
}

static void get_next_cluster(uint16_t* next_cluster, uint16_t current_cluster)
{
    seek_to_fat_region(current_cluster);
    disk_read(sizeof(uint16_t), next_cluster);
}

bool file_read(const char* name, void* address)
{
    uint32_t index = find_entry(name);
    if(index == -1)
        return false;

    dir_entry_t entry;
    seek_to_root_region(index);
    disk_read(sizeof(dir_entry_t), (void*)&entry);

    uint32_t bytes_read = 0;
    uint32_t cluster_offset = 0;
    uint16_t cluster = entry.first_cluster;

    while(bytes_read != entry.file_size)
    {
        seek_to_data_region(cluster, 0);

        uint32_t chunk_length = entry.file_size;
        uint32_t bytes_left_in_cluster = mount_info.bytes_per_cluster - cluster_offset;

        // Ensure it reads in the boundary of the cluster
        if(chunk_length > bytes_left_in_cluster)
            chunk_length = bytes_left_in_cluster;

        // Ensure it reads int the boundary of the file
        if(chunk_length > (entry.file_size - bytes_read))
            chunk_length = (entry.file_size - bytes_read);

        disk_read(chunk_length, (void*)((uint8_t*)address + bytes_read));

        bytes_read += chunk_length;
        cluster_offset += chunk_length;

        if(cluster_offset == mount_info.bytes_per_cluster)
        {
            cluster_offset = 0;
            get_next_cluster(&cluster, cluster);
        }
    }

    return true;
}