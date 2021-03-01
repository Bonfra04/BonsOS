#include <filesystem/fat16.h>
#include <string.h>
#include <ctype.h>
#include "fat16_types.h"

#define SECTOR_SIZE 512

static file_system_t fsys_fat;
static fsys_interact_function read_disk;
static fsys_interact_function write_disk;
static size_t device;
static size_t lba_offset;

static mount_info_t mount_info;

static void dos_filename(const char* filename, char* dos_fname)
{
    if (!dos_fname || !filename)
        return;
    memset(dos_fname, ' ', 11);

    int i;
    for (i = 0; i < strlen(filename); i++)
    {
        if (filename[i] == '.' || i == 8)
            break;

        dos_fname[i] = toupper(filename[i]);
    }

    if (filename[i] == '.')
        for (int k = 0; k < 3; k++)
        {
            if (filename[i++])
                dos_fname[8 + k] = toupper(filename[i]);
        }
}

static file_t search_root(const char* filename)
{
    char dos_fname[12];
    dos_filename(filename, dos_fname);
    dos_fname[11] = '\0';

    dir_entry_t entries[mount_info.num_root_entries];
    
    bool success = read_disk(device, lba_offset + mount_info.root_offset, mount_info.root_size, (void*)&entries);
    if(!success)
    {
        file_t invalid;
        invalid.flags = FS_INVALID;
        return invalid;
    }

    for(int i = 0; i < mount_info.num_root_entries; i++)
    {
        char mimmo[12];
        strncpy(mimmo, (const char*)&entries[i].name, 11);
        mimmo[11] = '\0';
        if(!strcmp(dos_fname, (const char*)&entries[i].name))
        {
            file_t file;

            strcpy(file.name, filename);
            file.first_cluster = entries[i].cluster_number;
            file.length = entries[i].file_size;
            file.position = 0;
            file.sector = 0;
            file.cluster = 0;
            file.eof = false;
            file.error = false,

            file.flags = entries[i].flags == 0x10 ? FS_DIRECTORY : FS_FILE;
            
            return file;
        }
    }

    file_t invalid;
    invalid.flags = FS_INVALID;
    return invalid;
}

static file_t serch_sub_dir(file_t dir, const char* filename)
{
    char dos_fname[12];
    dos_filename(filename, dos_fname);
    dos_fname[11] = '\0';

    file_t invalid;
    invalid.flags = FS_INVALID;
    return invalid;
}

void fat16_init(char device_letter, size_t device_id, size_t offset, fsys_interact_function read_function, fsys_interact_function write_function)
{
    read_disk = read_function;
    write_disk = write_function;
    device = device_id;
    lba_offset = offset;

    strcpy(fsys_fat.name, "FAT12");
    fsys_fat.mount = fat16_mount;
    fsys_fat.open_file = fat16_open_file;
    fsys_fat.close_file = fat16_close_file;
    fsys_fat.read_file = fat16_read_file;
    fsys_fat.write_file = fat16_write_file;
    fsys_fat.get_position = fat16_get_position;

    fsys_register_file_system(&fsys_fat, device_letter);

    fat16_mount();
}

bool fat16_mount()
{
    bootsector_t bootsector;
    bool success = read_disk(device, lba_offset + 0, 1, (void*)&bootsector);
    if(!success)
        return false;

    mount_info.num_sectors = bootsector.bpb.total_sectors;
    mount_info.fat_offset = 1;
    mount_info.fat_size = bootsector.bpb.sectors_per_fat * bootsector.bpb.bytes_per_sector;
    mount_info.fat_entry_size = 8;
    mount_info.num_root_entries = bootsector.bpb.root_entries;
    mount_info.root_offset = (bootsector.bpb.number_of_fats * bootsector.bpb.sectors_per_fat) + bootsector.bpb.reserved_sectors;
    mount_info.root_size = (bootsector.bpb.root_entries * sizeof(dir_entry_t)) / bootsector.bpb.bytes_per_sector;
    mount_info.bytes_per_sector = bootsector.bpb.bytes_per_sector;
    mount_info.sectors_per_cluster = bootsector.bpb.sectors_per_cluster;
    mount_info.sectors_per_fat = bootsector.bpb.sectors_per_fat;
    mount_info.first_cluster_sector = mount_info.root_offset + mount_info.root_size;

    return true;
}

file_t fat16_open_file(const char* filename)
{
    char* path = (char*)filename;

    file_t invalid;
    invalid.flags = FS_INVALID;

    char* p = (char*)strchr(path, '/');

    if(!p) // in root dir
    {
        file_t file = search_root(path);

        if(file.flags == FS_FILE)
            return file;

        return invalid;
    }

    p++;
    bool inRoot = true;
    while(p)
    {
        file_t currentFile;

        size_t len = 0;
        while(p[len] != '/' && p[len] != '\0')
            len++;

        char pathname[len + 1];
        strncpy(pathname, p, len);
        pathname[len] = '\0';

        if(inRoot)
        {
            currentFile = search_root(pathname);
            inRoot = false;
        }
        else
            currentFile = serch_sub_dir(currentFile, pathname);

        if(currentFile.flags == FS_INVALID)
            return invalid;

        if(currentFile.flags == FS_FILE)
            return currentFile;

        p = (char*)strchr(p, '/');
        if(p)
            p++;
    }

    return invalid;
}

void fat16_close_file(file_t* file)
{
    if(file)
        file->flags = FS_INVALID;
}

static void advance_file(file_t* file, size_t amount)
{
    file->position += amount;
    if(file->position >= mount_info.bytes_per_sector)
    {
        file->sector += file->position / mount_info.bytes_per_sector;
        file->position %= mount_info.bytes_per_sector;
        if(file->sector >= mount_info.sectors_per_cluster)
        {
            file->cluster += file->sector / mount_info.sectors_per_cluster;
            file->sector %= mount_info.sectors_per_cluster;
        }
    }

    while(fat16_get_position(file) >= file->length)
    {
        file->eof = true;
        file->position--;
    }
}

size_t fat16_read_file(file_t* file, void* buffer, size_t length)
{
    if(!file)
        return 0;

    if(fat16_get_position(file) >= file->length)
        file->eof = true;
    
    if(file->eof)
        return FS_EOF;

    uint16_t FAT[mount_info.fat_size];
    if(!read_disk(device, lba_offset + mount_info.fat_offset * mount_info.sectors_per_cluster, mount_info.sectors_per_fat, (void*)&FAT))
    {
        file->error = true;
        return 0;
    }

    uint8_t internal_buffer[mount_info.bytes_per_sector];
    size_t advance = 0;

    while(length > 0)
    {
        size_t cluster = file->first_cluster;
        size_t bias = file->cluster;
        while(bias--)
            cluster = FAT[cluster];

        size_t lba = (cluster - 2) * mount_info.sectors_per_cluster + file->sector;
        if(!read_disk(device, lba_offset + mount_info.first_cluster_sector + lba, 1, (void*)&internal_buffer))
        {
            file->error = true;
            return advance;
        }

        size_t left = file->length - fat16_get_position(file);
        size_t readable = (mount_info.bytes_per_sector - file->position);
        readable = left < readable ? left : readable;
        size_t amount = length > readable ? readable : length;
        memcpy((uint8_t*)buffer + advance, (uint8_t*)&internal_buffer + file->position, amount);
        advance += amount;
        length -= amount;

        advance_file(file, amount);

        if(file->eof)
            break;
    }

    return advance;
}

size_t fat16_write_file(file_t* file, void* buffer, size_t length)
{
    if(!file)
        return -1;

    return -1;
    /*
    uint16_t FAT[mount_info.fat_size];
    read_disk(device, lba_offset + mount_info.fat_offset * mount_info.sectors_per_cluster, mount_info.sectors_per_fat, (void*)&FAT);

    uint8_t internal_buffer[mount_info.bytes_per_sector];
    size_t advance = 0;

    while(length > 0)
    {
        size_t cluster = file->cluster;
        size_t bias = file->cluster;
        while(bias--)
            cluster = FAT[cluster];
        size_t lba = (cluster - 2) * mount_info.sectors_per_cluster + file->sector;
        read_disk(device, lba_offset + mount_info.first_cluster_sector + lba, 1, (void*)&internal_buffer);

        size_t readable = (mount_info.bytes_per_sector - file->position);
        size_t amount = length > readable ? readable : length;
        memcpy((uint8_t*)&internal_buffer + file->position, (uint8_t*)buffer + advance, amount);
        write_disk(device, lba_offset + mount_info.first_cluster_sector + lba, 1, (void*)&internal_buffer);
        advance += amount;
        length -= amount;

        advance_file(file, amount);
    }

    return advance;
    */
}

size_t fat16_get_position(file_t* file)
{
    size_t sectors = file->cluster * mount_info.sectors_per_cluster + file->sector;
    return sectors * mount_info.bytes_per_sector + file->position;
}

bool fat16_set_position(file_t* file, size_t position)
{
    position = position > file->length ? file->length : position;
    
    file->sector = position % mount_info.bytes_per_sector;
    position /= mount_info.bytes_per_sector;
    file->sector = position % mount_info.sectors_per_cluster;
    position /= mount_info.sectors_per_cluster;
    file->cluster = position;
}