#include <fsys/fsys.h>

#include <string.h>
#include <ctype.h>

#include "fat16_types.h"

void from_dos(char dos[8+3], char* name)
{
    uint64_t advance = 0;

    for(int i = 0; i < 8; i++)
        if(dos[i] != ' ')
            name[advance++] = tolower(dos[i]);
        else
            break;

    if(dos[8] != ' ')
        name[advance++] = '.';

    for(int i = 8; i < 11; i++)
        if(dos[i] != ' ')
            name[advance++] = tolower(dos[i]);
        else
            break;

    name[advance] = '\0';
}

file_t convert_entry(fat16_entry_t entry)
{
    file_t file;
    memset(&file, 0, sizeof(file_t));
    
    *(fat16_entry_t*)(&file.fs_data) = entry;

    return file;
}

fat16_entry_t* convert_file(file_t* file)
{
    return (fat16_entry_t*)file->fs_data;
}

bool get_next_cluster(fat16_data_t* data, uint64_t current_cluster, uint64_t* next_cluster)
{
    uint64_t addr = data->fat_offset + current_cluster * sizeof(uint16_t);
    
    if (current_cluster >= 0xFFF8 && current_cluster <= 0xFFFF)
        return false;

    if(!storage_seek(data->storage_id, data->offset + addr))
        return false;
    if(!storage_read(data->storage_id, sizeof(uint16_t), next_cluster))
        return false;

    return true;
}

void direntry_to_fatentry(dir_entry_t* d, fat16_entry_t* entry)
{
    entry->first_cluster = d->first_cluster;
    entry->type = d->flags & ENTRY_DIRECTORY ? FAT16_DIR : FAT16_FILE;
    entry->first_cluster = d->first_cluster;
    entry->length = d->file_size;
    entry->cluster = d->first_cluster;
    entry->advance = 0;
    entry->bytes_read = 0;
}