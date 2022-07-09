#include <fsys/fsys.h>
#include <storage/storage.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "fat16_types.h"

bool internal_list_dir(fat16_data_t* data, fat16_entry_t* dir_entry, direntry_t* dirent)
{
    bool lng = false;

    dir_entry_t d;
    do {
        if(!read_dir(data, dir_entry, &d))
        {
            dir_entry->error = true;
            return false;
        }

        if(d.fullname[0] == '\0')
            return false;

        if(d.flags & ENTRY_LFN == ENTRY_LFN)
        {
            lng = true;

            uint64_t internal_advance = 0;
            lfn_entry_t* lfn = (lfn_entry_t*)&d;

            // TODO: wchar
            for(uint8_t i = 0; i < 5; i++)
                if(lfn->name0[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * ((lfn->order & 0xF) - 1) + internal_advance++] = lfn->name0[i] & 0xFF;
            for(uint8_t i = 0; i < 6; i++)
                if(lfn->name1[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * ((lfn->order & 0xF) - 1) + internal_advance++] = lfn->name1[i] & 0xFF;
            for(uint8_t i = 0; i < 2; i++)
                if(lfn->name2[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * ((lfn->order & 0xF) - 1) + internal_advance++] = lfn->name2[i] & 0xFF;
        
            if(lfn->order & 0xF0 == 0x40)
                dirent->name[CHARS_PER_LFN * ((lfn->order & 0xF) - 1) + internal_advance] = '\0';
        }

    } while(d.flags & ENTRY_LFN == ENTRY_LFN);

    if(!lng)
    {
        char buffer[8+1+3+1];
        from_dos(d.fullname, buffer);
        strcpy(dirent->name, buffer);
    }
    
    dirent->flags = d.flags & ENTRY_DIRECTORY ? FSYS_FLG_DIR : FSYS_FLG_FILE; // TODO: better way to do this?

    *(dir_entry_t*)(&dirent->fs_data) = d;

    return true;
}

fat16_entry_t get_entry(fat16_data_t* data, fat16_entry_t* directory, const char* filename)
{
    fat16_entry_t dir = *directory;
    dir.error = false;

    char* fname = malloc(strlen(filename) + 1);
    strcpy(fname, filename);

    char* name = strtok(fname, "/");
    while(name != NULL)
    {
        if(dir.type != FAT16_DIR)
        {
            free(fname);
            return INVALID_ENTRY;
        }

        direntry_t dirent;
        while(internal_list_dir(data, &dir, &dirent))
        {
            if(strcmp(dirent.name, name) == 0)
            {
                direntry_to_fatentry((dir_entry_t*)dirent.fs_data, &dir);
                break;
            }
        }

        name = strtok(NULL, "/");
    }

    free(fname);
    return dir;
}

size_t read_entry(fat16_data_t* data, fat16_entry_t* entry, void* buffer, size_t length)
{
    uint64_t bytes_read = 0;

    if((entry->cluster >= 0xFFF8 && entry->cluster <= 0xFFFF) || (entry->bytes_read >= entry->length && entry->type == FAT16_FILE))
        return 0;

    while(length > 0)
    {
        size_t bytes_left_in_cluster = data->bytes_per_cluster - entry->advance;
        size_t chunk_length = length;
        if(chunk_length > bytes_left_in_cluster)
            chunk_length = bytes_left_in_cluster;
        if(entry->type == FAT16_FILE && chunk_length > entry->length - entry->advance)
            chunk_length = entry->length - entry->advance;

        uint64_t addr = (entry->cluster - 2) * data->bytes_per_cluster + data->first_data_sector + entry->advance;
        if(!storage_seek(data->storage_id, data->offset + addr))
        {
            entry->error = true;
            break;
        }

        if(storage_read(data->storage_id, chunk_length, buffer) != chunk_length)
        {
            entry->error = true;
            break;
        }

        entry->advance += chunk_length;
        entry->bytes_read += chunk_length;

        if(entry->advance >= data->bytes_per_cluster)
        {
            entry->advance = 0;
            if(!get_next_cluster(data, entry->cluster, &entry->cluster))
            {
                entry->error = true;
                break;
            }
        }

        length -= chunk_length;
        bytes_read += chunk_length;
        buffer = (uint8_t*)buffer + chunk_length;

        if(entry->bytes_read == entry->length && entry->type == FAT16_FILE)
            break;
    }

    return bytes_read;
}

bool read_dir(fat16_data_t* data, fat16_entry_t* dir, dir_entry_t* entry)
{
    if(read_entry(data, dir, entry, sizeof(dir_entry_t)) != sizeof(dir_entry_t))
        return false;
    return true;
}