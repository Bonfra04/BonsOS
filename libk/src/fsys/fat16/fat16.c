#include <fsys/fat16/fat16.h>
#include <storage/storage.h>
#include <log.h>

#include <string.h>
#include <assert.h>

#include "fat16_types.h"

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
    fs.error = fat16_error;
    fs.eof = fat16_eof;
    fs.clear_error = fat16_clear_error;

    bios_parameter_block_t* bpb = &bootsector.bpb;

    uint64_t root_offset = ((bpb->number_of_fats * bpb->sectors_per_fat) + bpb->reserved_sectors) * bpb->bytes_per_sector;

    fat16_data_t* data = (fat16_data_t*)&fs.data.fs_specific;
    data->offset = partition.start;
    data->storage_id = partition.device_id;
    data->bytes_per_cluster = bpb->sectors_per_cluster * bpb->bytes_per_sector;
    data->first_data_sector = root_offset + bpb->root_entries * sizeof(dir_entry_t);
    data->fat_offset = bpb->reserved_sectors * bpb->bytes_per_sector;

    fat16_entry_t root_dir;
    root_dir.first_cluster = (root_offset - data->first_data_sector) / data->bytes_per_cluster + 2;
    root_dir.type = FAT16_DIR;
    root_dir.advance = 0;
    root_dir.error = 0;
    root_dir.bytes_read = 0;
    root_dir.cluster = root_dir.first_cluster;
    root_dir.length = 0;

    data->root_dir = root_dir;

    return fs;
}

file_t fat16_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode)
{
    if(filename[0] != '/')
        return INVALID_FILE;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;

    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);
    if(entry.error || entry.type != FAT16_FILE)
        return INVALID_FILE;

    return convert_entry(entry);
}

bool fat16_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;

    *file = convert_entry(INVALID_ENTRY);
    return true;
}

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t* entry = convert_file(file);

    if(entry->error || entry->type != FAT16_FILE)
        return 0;

    return read_entry(data, entry, buffer, length);
}

size_t fat16_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t* entry = convert_file(file);

    if(entry->error || entry->type != FAT16_FILE)
        return 0;

    return write_entry(data, entry, buffer, length);
}

bool fat16_create_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;
 
    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = create_entry(data, &data->root_dir, filename + 1);
    
    if(entry.error)
        return false;

    return populate_file_entry(data, &entry);
}

bool fat16_delete_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);
    if(entry.error || entry.type != FAT16_FILE)
        return false;

    return remove_entry(data, &entry);
}

bool fat16_create_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = create_entry(data, &data->root_dir, dirpath + 1);
    
    if(entry.error)
        return false;

    return populate_dir_entry(data, &entry);
}

bool fat16_delete_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);
    if(entry.error || entry.type != FAT16_DIR)
        return false;

    return remove_entry(data, &entry);
}

size_t fat16_get_position(fs_data_t* fs, file_t* file)
{
    fat16_entry_t* entry = convert_file(file);
    if(entry->error || entry->type != FAT16_FILE)
        return 0;

    return entry->bytes_read;
}

bool fat16_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t* entry = convert_file(file);
    if(entry->type != FAT16_FILE)
        return 0;

    if(position > entry->length)
        position = entry->length;

    uint64_t nclusters = position / data->bytes_per_cluster;
    uint64_t offset = position % data->bytes_per_cluster;

    entry->cluster = entry->first_cluster;

    while(nclusters--)
        if(!get_next_cluster(data, entry->cluster, &entry->cluster))
        {
            entry->error = true;
            return false;
        }

    entry->advance = offset;
    entry->bytes_read = position;

    entry->error = false;

    return true;
}

bool fat16_exists_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, filename + 1);

    return entry.error == false && entry.type == FAT16_FILE;
}

bool fat16_exists_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return false;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);

    return entry.error == false && entry.type == FAT16_DIR;
}

file_t fat16_open_dir(fs_data_t* fs, const char* dirpath)
{
    if(dirpath[0] != '/')
        return INVALID_FILE;

    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;

    fat16_entry_t entry = get_entry(data, &data->root_dir, dirpath + 1);
    if(entry.type != FAT16_DIR)
        return INVALID_FILE;

    return convert_entry(entry);
}

bool fat16_list_dir(fs_data_t* fs, file_t* dir, direntry_t* dirent)
{
    fat16_data_t* data = (fat16_data_t*)&fs->fs_specific;
    fat16_entry_t* dir_entry = convert_file(dir);
    if(dir_entry->error || dir_entry->type != FAT16_DIR)
        return false;

    return internal_list_dir(data, dir_entry, dirent);
}

bool fat16_close_dir(fs_data_t* fs, file_t* dir)
{
    (void)fs;

    *dir = convert_entry(INVALID_ENTRY);
    return true;
}

bool fat16_error(fs_data_t* fs, file_t* file)
{
    fat16_entry_t* entry = convert_file(file);
    return entry->error;
}

bool fat16_eof(fs_data_t* fs, file_t* file)
{
    fat16_entry_t* entry = convert_file(file);
    return entry->bytes_read >= entry->length;
}

void fat16_clear_error(fs_data_t* fs, file_t* file)
{
    fat16_entry_t* entry = convert_file(file);
    entry->error = false;
}
