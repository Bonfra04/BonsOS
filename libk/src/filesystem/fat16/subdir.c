#include "subdir.h"
#include "fat16_types.h"

static size_t find_entry(fs_data_t* fs, file_t* dir, const char* entryname)
{
    /*
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    uint8_t rootdir[mount_info->root_size_in_bytes];
    if(!reaad_root(fs, (void*)&rootdir))
        return -1;
    
    dir_entry_t* entries = (dir_entry_t*)&rootdir;
    for(uint32_t i = 0; i < mount_info->num_root_entries; i++)
    {
        if(entries[i].flags & FREE)
            continue;

        if(!entries[i].fullname[0])
            break;
        
        // Ignore VFAT entries
        if(entries[i].flags & VFAT_DIR_ENTRY == VFAT_DIR_ENTRY)
            continue;

        if(!memcmp(entryname, entries[i].fullname, sizeof(entries[i].fullname)))
            return i;
    }

    return -1;
    */
}

file_t open_entry(fs_data_t* fs, file_t* dir, const char* filename, const char* mode, bool is_file)
{

}

file_t subdir_open_file(fs_data_t* fs, file_t* dir, const char* filename, const char* mode)
{
    return open_entry(fs, dir, filename, mode, true);
}

file_t subdir_open_dir(fs_data_t* fs, file_t* dir, const char* filename)
{
    return open_entry(fs, dir, filename, 'r', false);
}