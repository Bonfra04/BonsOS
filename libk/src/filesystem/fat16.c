#include <filesystem/fat16.h>
#include <string.h>
#include <ctype.h>
#include "fat16_types.h"

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

static fat16_file_t search_root(fs_data_t* fs, const char* filename)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    char dos_fname[12];
    dos_filename(filename, dos_fname);
    dos_fname[11] = '\0';

    dir_entry_t entries[mount_info->num_root_entries];

    if(!fs->reader(fs->device, fs->offset + mount_info->root_offset, mount_info->root_size, (void*)&entries))
    {
        fat16_file_t invalid;
        invalid.file.flags = FS_INVALID;
        return invalid;
    }

    for(int i = 0; i < mount_info->num_root_entries; i++)
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
            
            fat16_file_t fat16_file;
            fat16_file.file = file;
            fat16_file.dir_lba = mount_info->root_offset;
            fat16_file.dir_index = i;

            return fat16_file;
        }
    }

    fat16_file_t invalid;
    invalid.file.flags = FS_INVALID;
    return invalid;
}

static fat16_file_t serch_sub_dir(fs_data_t* fs, fat16_file_t dir, const char* filename)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);
    char dos_fname[12];
    dos_filename(filename, dos_fname);
    dos_fname[11] = '\0';

    while(!dir.file.eof)
    {
        uint8_t buff[mount_info->bytes_per_cluster];
        if(!fs->reader(fs->device, fs->offset + mount_info->first_cluster_sector + (dir.file.first_cluster - 2) * mount_info->sectors_per_cluster, mount_info->sectors_per_cluster, (void*)&buff))
            break;

        dir_entry_t* subdir = (dir_entry_t*)buff;
        for(size_t i = 0; i < mount_info->bytes_per_cluster / sizeof(dir_entry_t); i++)
        {
            char name[12];
			memcpy(name, subdir->name, 11);
			name[11] = 0;
            if(!name[0])
                goto invalid;

            if(!strcmp(dos_fname, name))
            {
                file_t file;

                strcpy(file.name, filename);
                file.first_cluster = subdir->cluster_number;
                file.length = subdir->file_size;
                file.position = 0;
                file.sector = 0;
                file.cluster = 0;
                file.eof = false;
                file.error = false,

                file.flags = subdir->flags == 0x10 ? FS_DIRECTORY : FS_FILE;

                fat16_file_t fat16_file;
                fat16_file.file = file;
                fat16_file.dir_lba = mount_info->first_cluster_sector + (dir.file.first_cluster - 2) * mount_info->sectors_per_cluster;
                fat16_file.dir_index = i;

                return fat16_file;
            }

            subdir++;
        }

    }

    invalid:
    {
        fat16_file_t invalid;
        invalid.file.flags = FS_INVALID;
        return invalid;
    }
}

file_system_t fat16_generate(size_t device, size_t offset, size_t size, fsys_interact_function disk_read, fsys_interact_function disk_write)
{
    file_system_t fs;
    strcpy(fs.data.name, "FAT16");
    fs.data.reader = disk_read;
    fs.data.writer = disk_write;
    fs.data.device = device;
    fs.data.offset = offset;
    fs.data.size = size;

    fs.mount = fat16_mount;
    fs.open_file = fat16_open_file;
    fs.close_file = fat16_close_file;
    fs.read_file = fat16_read_file;
    fs.write_file = fat16_write_file;
    fs.get_position = fat16_get_position;
    fs.set_position = fat16_set_position;
    fs.delete_file = fat16_delete_file;

    fs.mount(&(fs.data));

    return fs;
}

bool fat16_mount(fs_data_t* fs)
{
    bootsector_t bootsector;
    if(!fs->reader(fs->device, fs->offset + 0, 1, (void*)&bootsector))
        return false;

    memset(&(fs->fs_specific), 0, 512);
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    mount_info->num_sectors = bootsector.bpb.total_sectors;
    mount_info->fat_offset = 1;
    mount_info->fat_size = bootsector.bpb.sectors_per_fat * bootsector.bpb.bytes_per_sector;
    mount_info->fat_entry_size = 8;
    mount_info->num_root_entries = bootsector.bpb.root_entries;
    mount_info->root_offset = (bootsector.bpb.number_of_fats * bootsector.bpb.sectors_per_fat) + bootsector.bpb.reserved_sectors;
    mount_info->root_size = (bootsector.bpb.root_entries * sizeof(dir_entry_t)) / bootsector.bpb.bytes_per_sector;
    mount_info->bytes_per_sector = bootsector.bpb.bytes_per_sector;
    mount_info->sectors_per_cluster = bootsector.bpb.sectors_per_cluster;
    mount_info->sectors_per_fat = bootsector.bpb.sectors_per_fat;
    mount_info->first_cluster_sector = mount_info->root_offset + mount_info->root_size;
    mount_info->bytes_per_cluster = mount_info->bytes_per_sector * mount_info->sectors_per_cluster;

    return true;
}

file_t fat16_open_file(fs_data_t* fs, const char* filename)
{
    char* path = (char*)filename;

    file_t invalid;
    invalid.flags = FS_INVALID;

    char* p = (char*)strchr(path, '/');

    if(!p) // in root dir
    {
        file_t file = search_root(fs, path).file;

        if(file.flags == FS_FILE)
            return file;

        return invalid;
    }

    p++;
    bool inRoot = true;
    while(p)
    {
        fat16_file_t currentFile;

        size_t len = 0;
        while(p[len] != '/' && p[len] != '\0')
            len++;

        char pathname[len + 1];
        strncpy(pathname, p, len);
        pathname[len] = '\0';

        if(inRoot)
        {
            currentFile = search_root(fs, pathname);
            inRoot = false;
        }
        else
            currentFile = serch_sub_dir(fs, currentFile, pathname);

        if(currentFile.file.flags == FS_INVALID)
            return invalid;

        if(currentFile.file.flags == FS_FILE)
            return currentFile.file;

        p = (char*)strchr(p, '/');
        if(p)
            p++;
    }

    return invalid;
}

void fat16_close_file(fs_data_t* fs, file_t* file)
{
    if(file)
        file->flags = FS_INVALID;
}

static void advance_file(fs_data_t* fs, file_t* file, size_t amount)
{
    mount_info_t* mount_info = &(fs->fs_specific);

    file->position += amount;
    if(file->position >= mount_info->bytes_per_sector)
    {
        file->sector += file->position / mount_info->bytes_per_sector;
        file->position %= mount_info->bytes_per_sector;
        if(file->sector >= mount_info->sectors_per_cluster)
        {
            file->cluster += file->sector / mount_info->sectors_per_cluster;
            file->sector %= mount_info->sectors_per_cluster;
        }
    }

    while(fat16_get_position(fs, file) >= file->length)
    {
        file->eof = true;
        file->position--;
    }
}

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    mount_info_t* mount_info = &(fs->fs_specific);

    if(!file)
        return 0;

    if(fat16_get_position(fs, file) >= file->length)
        file->eof = true;

    if(file->eof)
        return FS_EOF;

    uint16_t FAT[mount_info->fat_size];
    if(!fs->reader(fs->device, fs->offset + mount_info->fat_offset * mount_info->sectors_per_cluster, mount_info->sectors_per_fat, (void*)&FAT))
    {
        file->error = true;
        return 0;
    }

    uint8_t internal_buffer[mount_info->bytes_per_sector];
    size_t advance = 0;

    while(length > 0)
    {
        size_t cluster = file->first_cluster;
        size_t bias = file->cluster;
        while(bias--)
            cluster = FAT[cluster];

        size_t lba = (cluster - 2) * mount_info->sectors_per_cluster + file->sector;
        if(!fs->reader(fs->device, fs->offset + mount_info->first_cluster_sector + lba, 1, (void*)&internal_buffer))
        {
            file->error = true;
            return advance;
        }

        size_t left = file->length - fat16_get_position(fs, file);
        size_t readable = (mount_info->bytes_per_sector - file->position);
        readable = left < readable ? left : readable;
        size_t amount = length > readable ? readable : length;
        memcpy((uint8_t*)buffer + advance, (uint8_t*)&internal_buffer + file->position, amount);
        advance += amount;
        length -= amount;

        advance_file(fs, file, amount);

        if(file->eof)
            break;
    }

    return advance;
}

size_t fat16_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
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

size_t fat16_get_position(fs_data_t* fs, file_t* file)
{
    mount_info_t* mount_info = &(fs->fs_specific);
    size_t sectors = file->cluster * mount_info->sectors_per_cluster + file->sector;
    size_t res = sectors * mount_info->bytes_per_sector + file->position;
    return res;
}

void fat16_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    mount_info_t* mount_info = &(fs->fs_specific);

    position = position > file->length ? file->length : position;

    file->position = position % mount_info->bytes_per_sector;
    position /= mount_info->bytes_per_sector;
    file->sector = position % mount_info->sectors_per_cluster;
    position /= mount_info->sectors_per_cluster;
    file->cluster = position;

    while(fat16_get_position(fs, file) > file->length)
        file->position--;
    file->eof = fat16_get_position(fs, file) == file->length;
}

bool fat16_delete_file(fs_data_t* fs, const char* filename)
{
    mount_info_t* mount_info = &(fs->fs_specific);

    char* path = (char*)filename;
    char* p = (char*)strchr(path, '/');

    fat16_file_t file;

    if(!p) // in root dir
    {
        file = search_root(fs, path);
        if(file.file.flags != FS_FILE)
            return false;
    }
    else
    {
        p++;
        bool inRoot = true;
        while(p)
        {
            fat16_file_t currentFile;

            size_t len = 0;
            while(p[len] != '/' && p[len] != '\0')
                len++;

            char pathname[len + 1];
            strncpy(pathname, p, len);
            pathname[len] = '\0';

            if(inRoot)
            {
                currentFile = search_root(fs, pathname);
                inRoot = false;
            }
            else
                currentFile = serch_sub_dir(fs, currentFile, pathname);

            if(currentFile.file.flags == FS_INVALID)
                return false;

            if(currentFile.file.flags == FS_FILE)
            {
                file = currentFile;
                break;
            }

            p = (char*)strchr(p, '/');
            if(p)
                p++;
        }
    }

    uint8_t buff[mount_info->bytes_per_cluster];
    if(!fs->reader(fs->device, fs->offset + file.dir_lba, mount_info->sectors_per_cluster, (void*)&buff))
        return false;

    dir_entry_t* entries = (dir_entry_t*)buff;
    for(size_t i = file.dir_index; i < mount_info->bytes_per_cluster / sizeof(dir_entry_t); i++)
    {
        entries[i] = entries[i + 1];
        if(!entries[i].name[0])
            break;
    }
    if(!fs->writer(fs->device, fs->offset + file.dir_lba, mount_info->sectors_per_cluster, (void*)&buff))
        return false;

    return true;
}