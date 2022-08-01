#include <fsys/fat16/fat16.h>

#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>

#include "fat16_utils.h"

#include <log.h>

file_system_t fat16_instantiate(partition_descriptor_t partition)
{
    file_system_t fs;
    memset(&fs, 0, sizeof(file_system_t));

    bootsector_t bootsector;
    if(!storage_seek(partition.device_id, partition.start))
        return fs;
    if(storage_read(partition.device_id, sizeof(bootsector), &bootsector) != sizeof(bootsector))
        return fs;

    fs.open_file = fat16_open_file;
    fs.close_file = fat16_close_file;
    fs.read_file = fat16_read_file;
    fs.write_file = fat16_write_file;
    fs.create_file = fat16_create_file;
    fs.delete_file = fat16_delete_file;
    fs.create_dir = fat16_create_dir;
    fs.delete_dir = fat16_delete_dir;
    fs.get_position = fat16_get_position;
    fs.set_position = fat16_set_position;
    fs.exists_file = fat16_exists_file;
    fs.exists_dir = fat16_exists_dir;
    fs.list_dir = fat16_list_dir;
    fs.open_dir = fat16_open_dir;
    fs.close_dir = fat16_close_dir;
    bios_parameter_block_t* bpb = &bootsector.bpb;

    uint64_t root_offset = ((bpb->number_of_fats * bpb->sectors_per_fat) + bpb->reserved_sectors) * bpb->bytes_per_sector;

    fat16_data_t* data = (fat16_data_t*)&fs.data.fs_specific;
    data->offset = partition.start;
    data->storage_id = partition.device_id;
    data->bytes_per_cluster = bpb->sectors_per_cluster * bpb->bytes_per_sector;
    data->data_start = root_offset + bpb->root_entries * sizeof(dir_entry_t);
    data->fat_offset = bpb->reserved_sectors * bpb->bytes_per_sector;

    fat16_entry_t root_dir;
    root_dir.first_cluster = (root_offset - data->data_start) / data->bytes_per_cluster + 2;
    root_dir.type = FAT16_DIR;
    root_dir.cluster_offset = 0;
    root_dir.advance = 0;
    root_dir.cluster = root_dir.first_cluster;
    root_dir.length = 0;
    root_dir.entry_addr = 0;
    root_dir.lfn = false;

    data->root_dir = root_dir;

    uint64_t root_size_in_sectors = (bpb->root_entries * sizeof(dir_entry_t)) / bpb->bytes_per_sector;
    uint64_t num_sectors = bpb->total_sectors != 0 ? bpb->total_sectors : bpb->total_sectors_big;
    uint64_t data_sector_count = num_sectors - (bpb->reserved_sectors + (bpb->number_of_fats * bpb->sectors_per_fat) + root_size_in_sectors);
    data->data_cluster_count = data_sector_count / bpb->sectors_per_cluster;

    return fs;
}

bool fat16_create_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    fat16_data_t* data = data_from_fs(fs);

    char* fname = strdup(filename);
    const char* parent_name = dirname(fname);
    fat16_entry_t parent = get_entry(data, &data->root_dir, parent_name);
    free(fname);

    if(get_entry(data, &parent, basename(filename)).type != FAT16_ERROR_ENTRY)
        return false;

    return create_entry(data, &parent, basename(filename), ENTRY_FILE);
}

bool fat16_delete_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    fat16_data_t* data = data_from_fs(fs);

    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);
    if(entry.type != FAT16_FILE)
        return false;

    return free_entry(data, &entry);
}

file_t fat16_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode)
{
    if(filename[0] != '/')
        return pack_file(INVALID_ENTRY);

    fat16_data_t* data = data_from_fs(fs);

    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);

    if(mode == FSYS_READ)
    {
        if(entry.type != FAT16_FILE)
            return pack_file(INVALID_ENTRY);
    }
    else if(mode == FSYS_WRITE)
    {
        if(entry.type != FAT16_ERROR_ENTRY)
            if(entry.type == FAT16_DIR)
                return pack_file(INVALID_ENTRY);
            else if(!fat16_delete_file(fs, filename))
                return pack_file(INVALID_ENTRY);

        if(!fat16_create_file(fs, filename))
            return pack_file(INVALID_ENTRY);
        entry = get_entry(data, &data->root_dir, filename + 1);
    }
    else if(mode == FSYS_APPEND)
    {
        if(entry.type != FAT16_ERROR_ENTRY)
        {
            if(!fat16_create_file(fs, filename))
                return pack_file(INVALID_ENTRY);
            entry = get_entry(data, &data->root_dir, filename + 1);
        }
        if(!set_pos(data, &entry, entry.length))
            return pack_file(INVALID_ENTRY);
    }

    return pack_file(entry);
}

bool fat16_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;
    *file = pack_file(INVALID_ENTRY);
    return true;
}

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    fat16_data_t* data = data_from_fs(fs);
    fat16_entry_t* entry = unpack_file(file);

    if(entry->type != FAT16_FILE)
        return -1;

    return read_entry(data, entry, buffer, length);
}

size_t fat16_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length)
{
    fat16_data_t* data = data_from_fs(fs);
    fat16_entry_t* entry = unpack_file(file);

    if(entry->type != FAT16_FILE)
        return -1;

    return write_entry(data, entry, buffer, length);
}

bool fat16_exists_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);

    return entry.type == FAT16_FILE;
}

bool fat16_create_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = data_from_fs(fs);

    char* dname = strdup(dirpath);
    const char* parent_name = dirname(dname);
    fat16_entry_t parent = get_entry(data, &data->root_dir, parent_name);
    free(dname);

    if(get_entry(data, &parent, basename(dirpath)).type != FAT16_ERROR_ENTRY)
        return false;

    if(!create_entry(data, &parent, basename(dirpath), ENTRY_DIRECTORY))
        return false;

    fat16_entry_t entry = get_entry(data, &parent, basename(dirpath));

    dir_entry_t dot_entry = construct_dir_entry(ENTRY_DIRECTORY);
    memcpy(dot_entry.fullname, ".          ", 11);
    dot_entry.first_cluster = entry.first_cluster;

    dir_entry_t dotdot_entry = construct_dir_entry(ENTRY_DIRECTORY);
    memcpy(dotdot_entry.fullname, "..         ", 11);
    dotdot_entry.first_cluster = parent.first_cluster;

    if(write_entry(data, &entry, &dot_entry, sizeof(dir_entry_t)) != sizeof(dir_entry_t))
        return false;

    if(write_entry(data, &entry, &dotdot_entry, sizeof(dir_entry_t)) != sizeof(dir_entry_t))
        return false;

    return true;
}

bool fat16_delete_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = data_from_fs(fs);

    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);
    if(entry.type != FAT16_DIR)
        return false;

    direntry_t dirent;
    while(list_dir(data, &entry, &dirent))
    {
        if(strlen(dirent.name) == 1 && dirent.name[0] == '.')
            continue;
        if(strlen(dirent.name) == 2 && dirent.name[0] == '.' && dirent.name[1] == '.')
            continue;

        char entryname[strlen(dirpath) + strlen(dirent.name) + 2];
        memset(entryname, 0, sizeof(entryname));
        sprintf(entryname, "%s/%s", dirpath, dirent.name);

        if((dirent.flags & FSYS_FLG_FILE) == FSYS_FLG_FILE)
        {
            if(!fat16_delete_file(fs, entryname))
                return false;
        }
        else if((dirent.flags & FSYS_FLG_DIR) == FSYS_FLG_DIR)
        {
            if(!fat16_delete_dir(fs, entryname))
                return false;
        }
    }

    return free_entry(data, &entry);
}

file_t fat16_open_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return pack_file(INVALID_ENTRY);

    fat16_data_t* data = data_from_fs(fs);

    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);

    if(entry.type != FAT16_DIR)
        return pack_file(INVALID_ENTRY);
    
    return pack_file(entry);
}

bool fat16_close_dir(fs_data_t* fs, file_t* dir)
{
    (void)fs;
    *dir = pack_file(INVALID_ENTRY);
    return true;
}

bool fat16_list_dir(fs_data_t* fs, file_t* dir, direntry_t* dirent)
{
    fat16_data_t* data = data_from_fs(fs);
    fat16_entry_t* dir_entry = unpack_file(dir);
    if(dir_entry->type != FAT16_DIR)
        return false;

    bool* del = &((fat16_direntry_t*)dirent->fs_data)->deleted;
    *del = true;
    while(*del)
        if(!list_dir(data, dir_entry, dirent))
            return false;
    return true;
}

bool fat16_exists_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);

    return entry.type == FAT16_DIR;
}

size_t fat16_get_position(fs_data_t* fs, const file_t* file)
{
    (void)fs;

    return get_pos(unpack_file(file));
}

bool fat16_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    fat16_data_t* data = data_from_fs(fs);
    return set_pos(data, unpack_file(file), position);
}

