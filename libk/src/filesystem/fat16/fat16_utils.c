#include "fat16_utils.h"
#include "fat16_types.h"
#include "path.h"
#include "rootdir.h"
#include "subdir.h"
#include <storage/disk_manager.h>

bool free_cluster_chain(fs_data_t* fs, uint16_t cluster)
{
    if(cluster == 0)
        return true;

    while(true)
    {
        size_t pos_cluster = seek_to_fat_region(fs, cluster);
        uint16_t next_cluster;
        disk_manager_read(fs->disk_id, sizeof(uint16_t), &next_cluster);
        
        uint16_t free_cluster = 0;
        disk_manager_seek(fs->disk_id, pos_cluster);
        disk_manager_write(fs->disk_id, sizeof(uint16_t), &free_cluster);
        
        if(next_cluster >= 0xFFF8)
            break;
        cluster = next_cluster;
    }

    return true;
}

bool allocate_cluster(fs_data_t* fs, uint16_t* new_cluster, uint16_t previous_cluster)
{
    mount_info_t* mount_info = (mount_info_t*)&fs->fs_specific;

    uint16_t next_cluster = FIRST_CLUSTER_INDEX_IN_FAT;

    seek_to_fat_region(fs, next_cluster);

    for(; next_cluster < mount_info->data_cluster_count - FIRST_CLUSTER_INDEX_IN_FAT; next_cluster++)
    {
        uint16_t entry;
        disk_manager_read(fs->disk_id, sizeof(uint16_t), &entry);

        if(entry == 0)
        {
            entry = 0xFFFF;
            seek_to_fat_region(fs, next_cluster);
            disk_manager_write(fs->disk_id, sizeof(uint16_t), &entry);
            break;
        }
    }

    if(next_cluster == mount_info->data_cluster_count)
        return false;

    if(previous_cluster != 0)
    {
        seek_to_fat_region(fs, previous_cluster);
        disk_manager_write(fs->disk_id, sizeof(uint16_t), &next_cluster);
    }

    *new_cluster = next_cluster;
    return true;
}

bool get_next_cluster(fs_data_t* fs, uint16_t* next_cluster, uint16_t current_cluster)
{
    seek_to_fat_region(fs, current_cluster);
    disk_manager_read(fs->disk_id, sizeof(uint16_t), next_cluster);

    return true;
}

void seek_to_data_region(fs_data_t* fs, size_t cluster, uint16_t offset)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    size_t tmp = cluster - 2;
    tmp *= mount_info->bytes_per_cluster;

    size_t pos = fs->offset;
    pos += mount_info->first_cluster_sector;
    pos += tmp;
    pos += offset;
    disk_manager_seek(fs->disk_id, pos);
}

size_t seek_to_fat_region(fs_data_t* fs, size_t cluster)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    size_t pos = fs->offset;
    pos += mount_info->fat_offset;
    pos += cluster * 2;
    disk_manager_seek(fs->disk_id, pos);
    return pos;
}

size_t seek_to_root_region(fs_data_t* fs, size_t entry_index)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    size_t pos = fs->offset;
    pos += mount_info->root_offset;
    pos += entry_index * sizeof(dir_entry_t);
    disk_manager_seek(fs->disk_id, pos);
    return pos;
}

file_t navigate_subdir(fs_data_t* fs, const char* dirpath)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    char subdir_name[13];
    char dos_name[12];
    size_t index = 0;

    if(!get_subdir(subdir_name, &index, dirpath))
        return invalid;

    if(!to_short_filename(dos_name, subdir_name))
        return invalid;

    file_t subdir = rootdir_open_dir(fs, dos_name);
    if(subdir.flags == FS_INVALID || subdir.error)
        return invalid;

    while(subdir.flags != FS_INVALID && !subdir.error)
    {
        bool ret = get_subdir(subdir_name, &index, dirpath);

        if(!to_short_filename(dos_name, subdir_name))
            return invalid;

        if(!ret)
            break;

        subdir = subdir_open_dir(fs, &subdir, dos_name);
    }

    if(subdir.flags == FS_INVALID || subdir.error)
        return invalid;

    return subdir;
}