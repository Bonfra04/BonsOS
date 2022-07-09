#include <fsys/fsys.h>

#include <string.h>

#include "fat16_types.h"

size_t write_entry(fat16_data_t* data, fat16_entry_t* entry, void* buffer, size_t length)
{
    // TODO: write entry fat16
    return 0;
}

fat16_entry_t create_entry(fat16_data_t* data, fat16_entry_t* dir, const char* filename)
{
    if(!get_entry(data, dir, filename).error)
        return INVALID_ENTRY;

    return INVALID_ENTRY;
}

bool remove_single(fat16_data_t* data, fat16_entry_t* entry)
{
    
    return false;
}

bool remove_entry(fat16_data_t* data, fat16_entry_t* entry)
{
    if(entry->type == FAT16_DIR)
    {
        direntry_t child;

        while(internal_list_dir(data, entry, &child))
        {
            fat16_entry_t child_entry;
            direntry_to_fatentry((dir_entry_t*)child.fs_data, &child_entry);

            if(strcmp(child.name, ".") == 0 || strcmp(child.name, "..") == 0)
                continue;

            if(child_entry.type == FAT16_DIR)
            {
                if(!remove_entry(data, &child))
                    return false;
            }
            else
            {
                if(!remove_single(data, &child))
                    return false;
            }
        }
    }

    return remove_single(data, entry);
}

bool populate_file_entry(fat16_data_t* data, fat16_entry_t* entry)
{
    return false;
}

bool populate_dir_entry(fat16_data_t* data, fat16_entry_t* entry)
{
    return false;
}
