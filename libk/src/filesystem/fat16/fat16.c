#include <filesystem/fat16.h>
#include <storage/disk_manager.h>
#include <string.h>
#include <ctype.h>
#include "fat16_types.h"
#include "rootdir.h"
#include "path.h"
#include "fat16_utils.h"
#include "subdir.h"

file_system_t fat16_generate(size_t disk_id, size_t offset, size_t size)
{
    file_system_t fs;
    strcpy(fs.data.name, "FAT16");
    fs.data.disk_id = disk_id;
    fs.data.offset = offset;
    fs.data.size = size;

    fs.mount = fat16_mount;

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

    fs.list_dir = fat16_list_dir;

    fs.exists_file = fat16_exists_file;
    fs.exists_dir = fat16_exists_dir;

    fs.mount(&(fs.data));

    return fs;
}

bool fat16_mount(fs_data_t* fs)
{
    bootsector_t bootsector;
    if(!disk_manager_seek(fs->disk_id, fs->offset))
        return false;
    if(!disk_manager_read(fs->disk_id, 512, (void*)&bootsector))
        return false;

    bios_parameter_block_t* bpb = &(bootsector.bpb); 

    memset(&(fs->fs_specific), 0, 512);
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    mount_info->num_sectors = bpb->total_sectors != 0 ? bpb->total_sectors : bpb->total_sectors_big;
    mount_info->bytes_per_sector = bpb->bytes_per_sector;
    mount_info->fat_offset = bpb->reserved_sectors * mount_info->bytes_per_sector;
    mount_info->fat_size_in_sectors = bpb->sectors_per_fat;
    mount_info->fat_size_in_bytes = mount_info->fat_size_in_sectors * bpb->bytes_per_sector;
    mount_info->fat_entry_size = 2;
    mount_info->num_root_entries = bpb->root_entries;
    mount_info->root_offset = ((bpb->number_of_fats * bpb->sectors_per_fat) + bpb->reserved_sectors) * mount_info->bytes_per_sector;
    mount_info->root_size_in_sectors = (bpb->root_entries * sizeof(dir_entry_t)) / bpb->bytes_per_sector;
    mount_info->root_size_in_bytes = mount_info->root_size_in_sectors * bpb->bytes_per_sector;
    mount_info->sectors_per_cluster = bpb->sectors_per_cluster;
    mount_info->sectors_per_fat = bpb->sectors_per_fat;
    mount_info->first_cluster_sector = mount_info->root_offset + mount_info->root_size_in_bytes;
    mount_info->bytes_per_cluster = bpb->bytes_per_sector * bpb->sectors_per_cluster;
    mount_info->data_sector_count = mount_info->num_sectors - (bpb->reserved_sectors + (bpb->number_of_fats * bpb->sectors_per_fat) + mount_info->root_size_in_sectors);
    mount_info->data_cluster_count = mount_info->data_sector_count / bpb->sectors_per_cluster;
    mount_info->dir_entry_per_sector = mount_info->bytes_per_sector / sizeof(dir_entry_t);

    return true;
}

file_t fat16_open_file(fs_data_t* fs, const char* filename, const char* mode)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    if(!mode || !filename)
        return invalid;

    if(*mode != 'r' && *mode != 'w' && *mode != 'a')
        return invalid;
    if(mode[1] != '\0' && mode[1] != '+')
        return invalid;

    if(fat16_exists_dir(fs, filename))
        return invalid;

    if(*mode == 'w')
    {
        if(fat16_exists_file(fs, filename))
            if(!fat16_delete_file(fs, filename))
                return invalid;
        fat16_create_file(fs, filename);
    }
    else if(*mode == 'a')
    {
        if(!fat16_exists_file(fs, filename))
            fat16_create_file(fs, filename);
    }

    char fname[11];

    if(is_in_root(filename))
    {
        if(!to_short_filename(fname, filename))
            return invalid;
        return rootdir_open_file(fs, fname, mode);
    }
    else
    {
        file_t dir = navigate_subdir(fs, filename);
        if(dir.flags == FS_INVALID || dir.error)
            return invalid;

        char* file = get_filename(filename);
        if(!to_short_filename(fname, file))
            return invalid;

        return subdir_open_file(fs, &dir, fname, mode);
    }
}

bool fat16_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;
    
    if(!file)
        return false;

    memset(file, 0, sizeof(file));
    file->flags = FS_INVALID;
    return true;
}

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    if(!file || !buffer)
        return 0;
    
    if(length == 0)
        return 0;

    if(file->mode != 'r')
    {
        file->error = true;
        return 0;
    }

    file_data_t* data = (file_data_t*)&(file->data);

    if(data->cluster == 0 || data->bytes_left == 0)
    {
        file->eof = true;
        return 0;
    }

    size_t bytes_read_count = 0;

    while(length > 0)
    {
        seek_to_data_region(fs, data->cluster, data->offset);
        
        if(data->bytes_left == 0)
            return bytes_read_count;

        size_t chunk_length = length;
        size_t bytes_left_in_cluster = mount_info->bytes_per_cluster - data->offset;

        // Ensure it reads in the boundary of the cluster
        if(chunk_length > bytes_left_in_cluster)
            chunk_length = bytes_left_in_cluster;

        // Ensure it reads int the boundary of the file
        if(chunk_length > data->bytes_left)
            chunk_length = data->bytes_left;

        if(!disk_manager_read(fs->disk_id, chunk_length, (uint8_t*)buffer + bytes_read_count))
        {
            file->error = true;
            return bytes_read_count;
        }
        
        data->bytes_left -= chunk_length;
        data->offset += chunk_length;

        if(data->offset == mount_info->bytes_per_cluster)
        {
            data->offset = 0;
            if(data->bytes_left != 0)
            {
                if(!get_next_cluster(fs, (uint16_t*)&(data->cluster), data->cluster))
                    return 0;
            }
        }

        length -= chunk_length;
        bytes_read_count += chunk_length;
    }

    if(data->bytes_left == 0)
        file->eof = true;

    return bytes_read_count;
}

size_t fat16_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    if(!file || !buffer)
        return 0;
    
    if(length == 0)
        return 0;

    if(file->mode == 'r')
    {
        file->error = true;
        return 0;
    }

    file_data_t* data = (file_data_t*)&(file->data);

    seek_to_data_region(fs, data->cluster, data->offset);

    size_t bytes_written_count = 0;

    while(length > 0)
    {
        size_t chunk_length = length;
        size_t bytes_left_in_cluster = mount_info->bytes_per_cluster - data->offset;

        // Check if a new cluster must be allocated
        if(data->cluster == 0 || bytes_left_in_cluster == 0)
        {
            uint16_t new_cluster;
            if(!allocate_cluster(fs, &new_cluster, data->cluster))
            {
                file->error = true;
                return bytes_written_count;
            }

            // If the file was empty the directory entry must be updated
            if(data->cluster == 0)
            {
                disk_manager_seek(fs->disk_id, fs->offset + data->dir_entry_address + offsetof(dir_entry_t, first_cluster));
                disk_manager_write(fs->disk_id, 2, (void*)&new_cluster);
                data->first_cluster = new_cluster;
            }

            data->cluster = new_cluster;
            data->offset = 0;

            seek_to_data_region(fs, data->cluster, 0);
            bytes_left_in_cluster = mount_info->bytes_per_cluster;
        }

        // Ensure it reads in the boundary of the cluster
        if(chunk_length > bytes_left_in_cluster)
            chunk_length = bytes_left_in_cluster;

        if(!disk_manager_write(fs->disk_id, chunk_length, (uint8_t*)buffer + bytes_written_count))
            return bytes_written_count; 

        length -= chunk_length;
        bytes_written_count += chunk_length;
        data->offset += chunk_length;
    }

    // Update filesize in dir entry
    file->length += bytes_written_count;
    disk_manager_seek(fs->disk_id, fs->offset + data->dir_entry_address + offsetof(dir_entry_t, file_size));
    disk_manager_write(fs->disk_id, 4, (void*)&(file->length));
    
    return bytes_written_count;
}

bool fat16_create_file(fs_data_t* fs, const char* filename)
{
    if(!filename)
        return false;

    if(fat16_exists_file(fs, filename) || fat16_exists_dir(fs, filename))
        return false;

    char fname[11];

    if(is_in_root(filename))
    {
        if(!to_short_filename(fname, filename))
            return false;
        return rootdir_create_file(fs, fname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, filename);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(filename);
        if(!to_short_filename(fname, file))
            return false;

        return subdir_create_file(fs, &dir, fname);
    }
}

bool fat16_delete_file(fs_data_t* fs, const char* filename)
{
    if(!filename)
        return false;

    if(!fat16_exists_file(fs, filename))
        return false;

    char fname[11];

    if(is_in_root(filename))
    {
        if(!to_short_filename(fname, filename))
            return false;
        return rootdir_delete_file(fs, fname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, filename);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(filename);
        if(!to_short_filename(fname, file))
            return false;

        return subdir_delete_file(fs, &dir, fname);
    }
}

bool fat16_create_dir(fs_data_t* fs, const char* dirpath)
{
    if(!dirpath)
        return false;

    if(fat16_exists_file(fs, dirpath) || fat16_exists_dir(fs, dirpath))
        return false;

    char dname[11];

    if(is_in_root(dirpath))
    {
        if(!to_short_filename(dname, dirpath))
            return false;
        return rootdir_create_dir(fs, dname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, dirpath);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(dirpath);
        if(!to_short_filename(dname, file))
            return false;

        return subdir_create_dir(fs, &dir, dname);
    } 
}

bool fat16_delete_dir(fs_data_t* fs, const char* dirpath)
{
    if(!dirpath)
        return false;

    if(!fat16_exists_dir(fs, dirpath))
        return false;

    char dname[11];

    if(is_in_root(dirpath))
    {
        if(!to_short_filename(dname, dirpath))
            return false;
        return rootdir_delete_dir(fs, dname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, dirpath);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(dirpath);
        if(!to_short_filename(dname, file))
            return false;

        return subdir_delete_dir(fs, &dir, dname);
    }
}

size_t fat16_get_position(fs_data_t* fs, file_t* file)
{
    if(!file)
        return -1;

    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    file_data_t* data = (file_data_t*)&(file->data);

    size_t res = data->cluster - data->first_cluster;
    res *= mount_info->bytes_per_cluster;
    res += data->offset;

    return res;
}

void fat16_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    if(!file)
        return;

    mount_info_t* mount_info = (mount_info_t*)&(fs->fs_specific);

    file_data_t* data = (file_data_t*)&(file->data);

    data->offset = position % mount_info->bytes_per_cluster;
    data->cluster = data->first_cluster + position / mount_info->bytes_per_cluster;
}

bool fat16_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath)
{
}

bool fat16_exists_file(fs_data_t* fs, const char* filename)
{
    char fname[11];

    if(is_in_root(filename))
    {
        if(!to_short_filename(fname, filename))
            return false;
        return rootdir_exists_file(fs, fname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, filename);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(filename);
        if(!to_short_filename(fname, file))
            return false;

        return subdir_exists_file(fs, &dir, fname);
    }
}

bool fat16_exists_dir(fs_data_t* fs, const char* dirpath)
{
    char dname[11];

    if(is_in_root(dirpath))
    {
        if(!to_short_filename(dname, dirpath))
            return false;
        
        return rootdir_exists_dir(fs, dname);
    }
    else
    {
        file_t dir = navigate_subdir(fs, dirpath);
        if(dir.flags == FS_INVALID || dir.error)
            return false;

        char* file = get_filename(dirpath);
        if(!to_short_filename(dname, file))
            return false;

        return subdir_exists_dir(fs, &dir, dname);
    }
}