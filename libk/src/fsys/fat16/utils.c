#include <storage/storage.h>

#include <string.h>
#include <ctype.h>

#include "fat16_utils.h"

fat16_entry_t* unpack_file(const file_t* file)
{
    return (fat16_entry_t*)file->fs_data;
}

file_t pack_file(fat16_entry_t entry)
{
    file_t file;
    memset(&file, 0, sizeof(file_t));
    *(fat16_entry_t*)(&file.fs_data) = entry;
    return file;
}

bool get_next_cluster(const fat16_data_t* data, uint64_t current_cluster, uint64_t* next_cluster)
{
    uint64_t addr = data->fat_offset + current_cluster * sizeof(uint16_t);
    
    *next_cluster = 0;
    if(storage_seek_read(data->storage_id, data->offset + addr, sizeof(uint16_t), next_cluster) != sizeof(uint16_t))
        return false;

    return !(next_cluster >= 0xFFF8 && next_cluster <= 0xFFFF);
}

void from_dos(char dos[8+3], char* name, uint8_t upplow_mask)
{
    uint64_t advance = 0;

    bool name_case = (upplow_mask >> 3) & 1;
    bool ext_case = (upplow_mask >> 4) & 1;

    for(int i = 0; i < 8; i++)
        if(dos[i] != ' ')
            name[advance++] = name_case ? tolower(dos[i]) : toupper(dos[i]);
        else
            break;

    if(dos[8] != ' ')
        name[advance++] = '.';

    for(int i = 8; i < 11; i++)
        if(dos[i] != ' ')
            name[advance++] = ext_case ? tolower(dos[i]) : toupper(dos[i]);
        else
            break;

    name[advance] = '\0';
}

size_t get_pos(const fat16_entry_t* entry)
{
    return entry->advance;
}

bool set_pos(const fat16_data_t* data, fat16_entry_t* entry, size_t position)
{
    if(entry->type == FAT16_FILE && position > entry->length)
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

    entry->cluster_offset = offset;
    entry->advance = position;

    entry->error = false;

    return true;
}

void direntry_to_fatentry(const direntry_t* d, fat16_entry_t* entry, uint64_t entry_addr)
{
    fat16_direntry_t dir = *(fat16_direntry_t*)(&d->fs_data);
    entry->first_cluster = dir.dir_entry.first_cluster;
    entry->type = dir.dir_entry.flags & ENTRY_DIRECTORY ? FAT16_DIR : FAT16_FILE;
    entry->length = dir.dir_entry.file_size;
    entry->cluster = dir.dir_entry.first_cluster;
    entry->cluster_offset = 0;
    entry->advance = 0;
    entry->error = false;
    entry->lfn = dir.lfn;
    entry->entry_addr = entry_addr;
}

bool allocate_cluster(const fat16_data_t* data, uint64_t current_cluster, uint64_t* new_cluster)
{
    if(!storage_seek(data->storage_id, data->offset + data->fat_offset))
        return false;

    uint16_t cluster = FIRST_CLUSTER_OFFSET;

    for(; cluster < data->data_cluster_count - FIRST_CLUSTER_OFFSET; cluster++)
    {
        uint16_t entry;
        if(storage_read(data->storage_id, sizeof(uint16_t), &entry) != sizeof(uint16_t))
            return false;

        if(entry == 0)
        {
            entry = 0xFFFF;
            if(storage_seek_write(data->storage_id, data->offset + data->fat_offset + cluster * sizeof(uint16_t), sizeof(uint16_t), &entry) != sizeof(uint16_t))
                return false;
            break;
        }
    }

    if(cluster == data->data_cluster_count - FIRST_CLUSTER_OFFSET)
        return false;

    if(current_cluster != 0)
        if(storage_seek_write(data->storage_id, data->offset + data->fat_offset + current_cluster * sizeof(uint16_t), sizeof(uint16_t), &cluster) != sizeof(uint16_t))
            return false;
    
    *new_cluster = cluster;
    return true;
}
