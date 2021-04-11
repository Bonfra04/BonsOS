#include "rootdir.h"
#include "fat16_types.h"
#include <string.h>
#include "fat16_utils.h"
#include <storage/disk_manager.h>>

static bool last_entry(fs_data_t* fs, size_t index)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    if(index == mount_info->num_root_entries - 1)
        return true;

    uint8_t tmp;
    seek_to_root_region(fs, index + 1);
    disk_manager_read(fs->disk_id, sizeof(uint8_t), &tmp);
    return !tmp;
}

static bool mark_entry_as_available(fs_data_t* fs, size_t entry_index)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));

    if(!last_entry(fs, entry_index))
        entry.flags |= FREE;

    seek_to_root_region(fs, entry_index);
    disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &entry);

    return true;
}

static size_t find_entry(fs_data_t* fs, const char* entryname)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    seek_to_root_region(fs, 0);
    
    for(size_t i = 0; i < mount_info->num_root_entries; i++)
    {
        dir_entry_t entry;
        disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

        if(entry.flags & FREE)
            continue;

        if(!entry.fullname[0])
            break;
        
        // Ignore VFAT entries
        if(entry.flags & VFAT_DIR_ENTRY)
            continue;

        if(!memcmp(entryname, entry.fullname, sizeof(entry.fullname)))
            return i;
    }

    return -1;
}

static size_t find_available_entry(fs_data_t* fs)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    size_t pos = seek_to_root_region(fs, 0);

    for(uint32_t i = 0; i < mount_info->num_root_entries; i++)
    {
        dir_entry_t entry;
        disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
        
        if(!(entry.fullname[0]) || entry.flags & FREE)
            return i;
    }

    return -1;
}

static file_t create_entry(fs_data_t* fs, const char* entryname, uint8_t flags)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    if(rootdir_exists_file(fs, entryname) || rootdir_exists_dir(fs, entryname))
        return invalid;

    size_t entry_index = find_available_entry(fs);
    if(entry_index == -1)
        return invalid;
    
    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));
    memcpy(entry.fullname, entryname, sizeof(entry.fullname));
    entry.flags = flags;

    seek_to_root_region(fs, entry_index);
    disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &entry);

    file_t file;
    file.error = false;
    file.eof = false;
    file.flags = FS_FILE;
    memcpy(file.name, entry.fullname, sizeof(entry.fullname));

    file_data_t* data = (file_data_t*)&(file.data);
    data->dir_entry_address = mount_info->root_offset;
    data->dir_entry_address += sizeof(dir_entry_t) * entry_index;

    return file;
}

static bool delete_entry(fs_data_t* fs, const char* entryname, bool is_file)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    size_t entry_index = find_entry(fs, entryname);
    if(entry_index == -1)
        return false;

    dir_entry_t entry;
    seek_to_root_region(fs, entry_index);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(entry.flags & VOLUME)
        return false;
    if(is_file && (entry.flags & SUBDIR) || !is_file && !(entry.flags & SUBDIR))
        return false;

    if(!mark_entry_as_available(fs, entry_index))
        return false;
    if(!free_cluster_chain(fs, entry.first_cluster))
        return false;

    return true;
}

file_t open_entry(fs_data_t* fs, const char* filename, const char* mode, bool is_file)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    size_t entry_index = find_entry(fs, filename);
    if(entry_index == -1)
        return invalid;

    dir_entry_t entry;
    seek_to_root_region(fs, entry_index);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(entry.flags & VOLUME)
        return invalid;
    if(is_file && entry.flags & SUBDIR)
        return invalid;

    if(entry.flags & READ_ONLY && (*mode != 'r' || mode[1] == '+'))
        return invalid;

    file_t file;
    file_data_t* data = (file_data_t*)&(file.data);
    memset(&file, 0, sizeof(file_t));
    file.mode = *mode;
    file.update = mode[1] == '+';
    file.length = entry.file_size;
    data->dir_index = entry_index;
    data->dir_entry_address = mount_info->root_offset;
    data->dir_entry_address += sizeof(dir_entry_t) * entry_index;

    data->cluster = entry.first_cluster;
    data->first_cluster = entry.first_cluster;

    // In append mode place the cursor at the end of the file
    if(*mode == 'a')
    {
        data->offset = entry.file_size;
        size_t next_clustet;
        do
        {
            if(!get_next_cluster(fs, (uint16_t*)&next_clustet, data->cluster))
                return invalid;
            data->cluster = next_clustet;
            data->offset -= mount_info->bytes_per_cluster;
        } while(next_clustet < 0xFFF8);
    }
    else
        data->offset = 0;

    if(*mode == 'r')
        data->bytes_left = entry.file_size;
    else
        data->bytes_left = 0;

    if(is_file)
        file.flags = FS_FILE;
    else
        file.flags = FS_DIRECTORY;

    return file;
}

file_t rootdir_open_file(fs_data_t* fs, const char* filename, const char* mode)
{
    return open_entry(fs, filename, mode, true);
}

file_t rootdir_create_file(fs_data_t* fs, const char* filename)
{
    return create_entry(fs, filename, 0);
}

file_t rootdir_create_dir(fs_data_t* fs, const char* dirpath)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    file_t dir = create_entry(fs, dirpath, SUBDIR);
    if(dir.flags == FS_INVALID)
        return invalid;
    file_data_t* data = (file_data_t*)&(dir.data);

    dir_entry_t entry;
    size_t pos = seek_to_root_region(fs, data->dir_index);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(!allocate_cluster(fs, &(entry.first_cluster), 0))
        return invalid;

    pos += offsetof(dir_entry_t, first_cluster);
    disk_manager_seek(fs->disk_id, pos);
    disk_manager_write(fs->disk_id, sizeof(entry.first_cluster), &entry.first_cluster);

    seek_to_data_region(fs, entry.first_cluster, 0);
    // Create "." entry
    {
        dir_entry_t dot_entry;
        memset(&dot_entry, 0, sizeof(dir_entry_t));

        memset(&(dot_entry.fullname), ' ', sizeof(dot_entry.fullname));
        dot_entry.fullname[0] = '.';
        dot_entry.flags = SUBDIR;
        dot_entry.first_cluster = entry.first_cluster;

        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &dot_entry);
    }

    // Create ".." entry
    {
        dir_entry_t prev_entry;
        memset(&prev_entry, 0, sizeof(dir_entry_t));

        memset(&(prev_entry.fullname), ' ', sizeof(prev_entry.fullname));
        prev_entry.fullname[0] = '.';
        prev_entry.fullname[1] = '.';
        prev_entry.flags = SUBDIR;

        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &prev_entry);
    }

    // Add a dummy empty dir to indicate end
    {
        dir_entry_t dummy_entry;
        memset(&dummy_entry, 0, sizeof(dir_entry_t));

        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &dummy_entry);
    }

    return dir;
}

bool rootdir_delete_file(fs_data_t* fs, const char* filename)
{
    return delete_entry(fs, filename, true);
}

bool rootdir_delete_dir(fs_data_t* fs, const char* dirpath)
{
    return delete_entry(fs, dirpath, false);
}

bool rootdir_exists_file(fs_data_t* fs, const char* filename)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    size_t entry_index = find_entry(fs, filename);
    if(entry_index == -1)
        return false;

    dir_entry_t entry;
    seek_to_root_region(fs, entry_index);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
    
    return !(entry.flags & SUBDIR);
}

bool rootdir_exists_dir(fs_data_t* fs, const char* dirpath)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    size_t entry_index = find_entry(fs, dirpath);
    if(entry_index == -1)
        return false;

    dir_entry_t entry;
    seek_to_root_region(fs, entry_index);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
    
    return entry.flags & SUBDIR;
}