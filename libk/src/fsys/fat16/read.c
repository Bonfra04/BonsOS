#include <storage/storage.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "fat16_utils.h"

size_t read_entry(const fat16_data_t* data, fat16_entry_t* entry, void* buffer, size_t length)
{
    uint64_t bytes_read = 0;

    while(length > 0)
    {
        if(entry->type == FAT16_FILE && entry->advance == entry->length)
            break;

        // read next cluster
        if(entry->cluster_offset == data->bytes_per_cluster)
        {
            entry->cluster_offset = 0;
            if(!get_next_cluster(data, entry->cluster, &entry->cluster))
                return -1;
        }

        // calculate to read length
        size_t chunk_length = ullmin(length, data->bytes_per_cluster - entry->cluster_offset);
        if(entry->type == FAT16_FILE)
            chunk_length = ullmin(chunk_length, entry->length - entry->advance);

        // read chunk
        uint64_t addr = get_entry_pos(data, entry);
        if(storage_seek_read(data->storage_id, data->offset + addr, chunk_length, buffer) != chunk_length)
            return -1;

        // adjust entry
        entry->cluster_offset += chunk_length;
        entry->advance += chunk_length;
        bytes_read += chunk_length;
        length -= chunk_length;
        buffer = (uint8_t*)buffer + chunk_length;
    }

    return bytes_read;
}

bool list_dir(const fat16_data_t* data, fat16_entry_t* dir, direntry_t* dirent)
{
    bool lng = false;
    bool del = false;

    dir_entry_t d;
    do {
        skip:
        if(read_entry(data, dir, &d, sizeof(dir_entry_t)) != sizeof(dir_entry_t))
            return false;

        if(d.fullname[0] == '\0')
        {
            memset(dirent, 0, sizeof(direntry_t));
            return false;
        }

        if((uint8_t)d.fullname[0] == ENTRY_DELETED)
            del = true;
        if(d.flags == ENTRY_VOLUME_ID)
            goto skip;

        if((d.flags & ENTRY_LFN) == ENTRY_LFN)
        {
            lng = true;

            uint64_t internal_advance = 0;
            lfn_entry_t* lfn = (lfn_entry_t*)&d;

            uint8_t order = ((lfn->order & 0xF) - 1);

            // TODO: wchar
            for(uint8_t i = 0; i < 5; i++)
                if(lfn->name0[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * order + internal_advance++] = lfn->name0[i] & 0xFF;
            for(uint8_t i = 0; i < 6; i++)
                if(lfn->name1[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * order + internal_advance++] = lfn->name1[i] & 0xFF;
            for(uint8_t i = 0; i < 2; i++)
                if(lfn->name2[i] != 0xFFFF)
                    dirent->name[CHARS_PER_LFN * order + internal_advance++] = lfn->name2[i] & 0xFF;

            if((lfn->order & 0xF0) == LFN_END_MASK)
                dirent->name[CHARS_PER_LFN * order + internal_advance] = '\0';
        }
    } while((d.flags & ENTRY_LFN) == ENTRY_LFN);

    if(!lng)
    {
        char buffer[8+1+3+1];
        from_dos(d.fullname, buffer, d.upplow_mask);
        strcpy(dirent->name, buffer);
    }

    dirent->flags = d.flags & ENTRY_DIRECTORY ? FSYS_FLG_DIR : FSYS_FLG_FILE;

    fat16_direntry_t fat16_dirent;
    fat16_dirent.dir_entry = d;
    fat16_dirent.lfn = lng;
    fat16_dirent.deleted = del;

    *(fat16_direntry_t*)(&dirent->fs_data) = fat16_dirent;

    return true;
}

fat16_entry_t get_entry(const fat16_data_t* data, const fat16_entry_t* directory, const char* filename)
{
    fat16_entry_t entry = *directory;

    char* fname = strdup(filename);
    char* name = strtok(fname, "/");
    while(name != NULL && name[0] != '\0')
    {
        if(entry.type != FAT16_DIR)
        {
            free(fname);
            return INVALID_ENTRY;
        }

        direntry_t dirent;
        memset(&dirent, 0, sizeof(direntry_t));
        while(list_dir(data, &entry, &dirent))
            if(strlen(dirent.name) == strlen(name) && strcmp(dirent.name, name) == 0)
            {
                uint64_t entry_addr = get_entry_pos(data, &entry) - sizeof(dir_entry_t);
                direntry_to_fatentry(&dirent, &entry, entry_addr);
                break;
            }
        if(dirent.name[0] == '\0')
        {
            free(fname);
            return INVALID_ENTRY;
        }

        name = strtok(NULL, "/");
    }

    free(fname);
    return entry;
}
