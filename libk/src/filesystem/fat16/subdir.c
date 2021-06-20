#include "subdir.h"
#include "fat16_types.h"
#include "fat16_utils.h"
#include <string.h>
#include <storage/disk_manager.h>

static dir_entry_t read_next_entry(fs_data_t* fs, file_t* dir)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);
    file_data_t* data = (file_data_t*)&(dir->data);

    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));

    // assume cluster size is multiple of dir_entry_t size
    if(data->offset == mount_info->bytes_per_cluster)
    {
        uint16_t next_cluster;
        if(!get_next_cluster(fs, &next_cluster, data->cluster))
            return entry;

        if(next_cluster > 0xFFF8)
            return entry;

        data->cluster = next_cluster;
        data->offset = 0;
    }

    seek_to_data_region(fs, data->cluster, data->offset);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
    data->offset += sizeof(dir_entry_t);
    return entry;
}

static size_t find_entry(fs_data_t* fs, file_t* dir, const char* entryname)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);
    file_data_t* data = (file_data_t*)&(dir->data);

    dir_entry_t entry;

    while(true)
    {
        entry = read_next_entry(fs, dir);
        
        if(entry.flags & FREE)
            continue;

        if(!entry.fullname[0])
            break;
        
        // Ignore VFAT entries
        if(entry.flags & VFAT_DIR_ENTRY)
            continue;

        if(!memcmp(entryname, entry.fullname, sizeof(entry.fullname)))
        {
            size_t cluster_offset = (data->cluster - data->first_cluster) * mount_info->bytes_per_cluster;
            cluster_offset /= sizeof(dir_entry_t);
            return cluster_offset + data->offset / sizeof(dir_entry_t) - 1;
        }
    }

    return -1;
}

static file_t open_entry(fs_data_t* fs, file_t* dir, const char* filename, const char* mode, bool is_file)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    size_t entry_index = find_entry(fs, dir, filename);
    if(entry_index == -1)
        return invalid;
    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    dir_entry_t entry;
    seek_to_data_region(fs, cluster, offset);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(entry.flags & VOLUME)
        return invalid;
    if(is_file && entry.flags & SUBDIR)
        return invalid;
    if(entry.flags & READ_ONLY && (*mode != 'r' || mode[1] == '+'))
        return invalid;

    size_t dir_address = mount_info->first_cluster_sector;
    dir_address += (((file_data_t*)&(dir->data))->first_cluster - 2) * mount_info->bytes_per_cluster;

    file_t file;
    file_data_t* data = (file_data_t*)&(file.data);
    memset(&file, 0, sizeof(file));
    memcpy(file.name, entry.fullname, sizeof(entry.fullname));
    file.mode = *mode;
    file.update = mode[1] == '+';
    file.length = entry.file_size;
    data->dir_entry_address = dir_address + entry_index * sizeof(dir_entry_t);

    data->cluster = entry.first_cluster;
    data->first_cluster = entry.first_cluster;

    // In append mode place the cursor at the end of the file
    if(*mode == 'a')
    {
        data->offset = entry.file_size;
        uint16_t next_cluster;
        do
        {
            if(!get_next_cluster(fs, &next_cluster, data->cluster))
                return invalid;
            data->cluster = next_cluster;
            data->offset -= mount_info->bytes_per_cluster;
        } while(next_cluster < 0xFFF8);
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

static bool last_entry(fs_data_t* fs, file_t* dir, size_t entry_index)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    uint8_t tmp;
    seek_to_data_region(fs, cluster, offset);
    disk_manager_read(fs->disk_id, sizeof(uint8_t), &tmp);
    return !tmp;
}

static bool mark_entry_as_available(fs_data_t* fs, file_t* dir, size_t entry_index)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));
    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    if(!last_entry(fs, dir, entry_index))
        entry.flags |= FREE;

    seek_to_data_region(fs, cluster, offset);
    disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &entry);

    return true;
}

static bool delete_entry(fs_data_t* fs, file_t* dir, const char* entryname, bool is_file)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    size_t entry_index = find_entry(fs, dir, entryname);
    if(entry_index == -1)
        return false;
    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    dir_entry_t entry;
    seek_to_data_region(fs, cluster, offset);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(entry.flags & VOLUME)
        return false;
    if(is_file && (entry.flags & SUBDIR) || !is_file && !(entry.flags & SUBDIR))
        return false;

    if(!mark_entry_as_available(fs, dir, entry_index))
        return false;
    if(!free_cluster_chain(fs, entry.first_cluster))
        return false;

    return true;
}

static size_t find_available_entry(fs_data_t* fs, file_t* dir)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);
    file_data_t* data = (file_data_t*)&(dir->data);

    dir_entry_t entry;
    size_t res = 0;
    while((entry = read_next_entry(fs, dir)).name[0])
    {
        if(entry.flags & FREE)
        {
            // restore dir to starting state
            data->cluster = data->first_cluster;
            data->offset = 0;
            return res;
        }
        res++;
    }

    // Add a dummy empty dir to indicate end
    {
        dir_entry_t dummy_entry;
        memset(&dummy_entry, 0, sizeof(dir_entry_t));

        if(data->offset == mount_info->bytes_per_cluster)
        {
            uint16_t new_cluster;
            allocate_cluster(fs, &new_cluster, data->cluster);
            data->cluster = new_cluster;
            data->offset = 0;
        }

        seek_to_data_region(fs, data->cluster, data->offset);
        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &dummy_entry);
    }

    // restore dir to starting state
    data->cluster = data->first_cluster;
    data->offset = 0;

    return res;
}

static bool create_entry(fs_data_t* fs, file_t* dir, const char* entryname, uint8_t flags)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* entry_data = (file_data_t*)&(dir->data);

    if(subdir_exists_file(fs, dir, entryname) || subdir_exists_dir(fs, dir, entryname))
        return false;

    size_t entry_index = find_available_entry(fs, dir);
    if(entry_index == -1)
        return false;

    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));
    memcpy(entry.fullname, entryname, sizeof(entry.fullname));
    entry.flags = flags;

    size_t cluster = entry_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    seek_to_data_region(fs, cluster, offset);
    disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &entry);

    return true; 
}

file_t subdir_open_file(fs_data_t* fs, file_t* dir, const char* filename, const char* mode)
{
    return open_entry(fs, dir, filename, mode, true);
}

file_t subdir_open_dir(fs_data_t* fs, file_t* dir, const char* filename)
{
    return open_entry(fs, dir, filename, "r", false);
}

bool subdir_create_file(fs_data_t* fs, file_t* dir, const char* filename)
{
    return create_entry(fs, dir, filename, 0);
}

bool subdir_create_dir(fs_data_t* fs, file_t* dir, const char* dirpath)
{
    if(!create_entry(fs, dir, dirpath, SUBDIR))
        return false;

    file_t subdir = subdir_open_dir(fs, dir, dirpath);
    file_data_t* data = (file_data_t*)&(subdir.data);

    dir_entry_t entry;
    disk_manager_seek(fs->disk_id, fs->offset + data->dir_entry_address);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);

    if(!allocate_cluster(fs, &(entry.first_cluster), 0))
        return false;

    size_t pos = fs->offset + data->dir_entry_address + offsetof(dir_entry_t, first_cluster);
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
        prev_entry.first_cluster = ((file_data_t*)&(dir->data))->first_cluster;

        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &prev_entry);
    }

    // Add a dummy empty dir to indicate end
    {
        dir_entry_t dummy_entry;
        memset(&dummy_entry, 0, sizeof(dir_entry_t));

        disk_manager_write(fs->disk_id, sizeof(dir_entry_t), &dummy_entry);
    }

    return true;
}

bool subdir_delete_file(fs_data_t* fs, file_t* dir, const char* filename)
{
    return delete_entry(fs, dir, filename, true);
}

bool subdir_delete_dir(fs_data_t* fs, file_t* dir, const char* dirpath)
{
    return delete_entry(fs, dir, dirpath, false);
}

bool subdir_exists_file(fs_data_t* fs, file_t* dir, const char* filename)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    size_t entry_index = find_entry(fs, dir, filename);
    if(entry_index == -1)
    {
        // restore dir to starting state
        dir_data->cluster = dir_data->first_cluster;
        dir_data->offset = 0;
        return false;
    }
    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    dir_entry_t entry;
    seek_to_data_region(fs, cluster, offset);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
    
    // restore dir to starting state
    dir_data->cluster = dir_data->first_cluster;
    dir_data->offset = 0;

    return !(entry.flags & SUBDIR);
}

bool subdir_exists_dir(fs_data_t* fs, file_t* dir, const char* dirpath)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;
    file_data_t* dir_data = (file_data_t*)&(dir->data);

    size_t entry_index = find_entry(fs, dir, dirpath);
    if(entry_index == -1)
    {
        // restore dir to starting state
        dir_data->cluster = dir_data->first_cluster;
        dir_data->offset = 0;
        return false;
    }
    size_t cluster = dir_data->first_cluster + entry_index * sizeof(dir_entry_t) / mount_info->bytes_per_cluster;
    size_t offset = entry_index * sizeof(dir_entry_t) % mount_info->bytes_per_cluster;

    dir_entry_t entry;
    seek_to_data_region(fs, cluster, offset);
    disk_manager_read(fs->disk_id, sizeof(dir_entry_t), &entry);
    
    // restore dir to starting state
    dir_data->cluster = dir_data->first_cluster;
    dir_data->offset = 0;

    return entry.flags & SUBDIR;
}