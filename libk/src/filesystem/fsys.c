#include <filesystem/fsys.h>
#include <device/ata/ahci.h>
#include <filesystem/fat16.h>

#define DEVICE_MAX 26

static file_system_t* file_systems[DEVICE_MAX];

file_system_t fsys_generate(uint8_t type, size_t disk_id, size_t offset, size_t size)
{
    switch (type)
    {
    case FSYS_FAT16:
        return fat16_generate(disk_id, offset, size);
    }
    
    file_system_t invalid = {0};
    return invalid;
}

void fsys_register_file_system(file_system_t* file_system, char device_letter)
{
    file_systems[device_letter - 'a'] = file_system;
}

void fsys_unregister_file_system(file_system_t* file_system)
{
    for (uint8_t i = 0; i < DEVICE_MAX; i++)
        if (file_systems[i] == file_system)
            file_systems[i] = 0;
}

file_t fsys_open_file(const char* filename, const char* mode)
{
    file_t invalid;
    invalid.flags = FS_INVALID;
    invalid.error = true;

    if(!filename)
        return invalid;

    char* fname = (char*)filename;
    char device = 'a';
    if (filename[1] == ':')
    {
        device = filename[0];
        fname += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return invalid;

    fs_data_t* data = &(fs->data);
    file_t file = fs->open_file(data, fname, mode);
    file.device = device;
    return file;
}

bool fsys_close_file(file_t* file)
{
    if (!file || file->flags != FS_FILE)
        return false;

    file_system_t* fs = file_systems[file->device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->close_file(data, file);
}

size_t fsys_read_file(file_t* file, void* buffer, size_t length)
{
    if (!file || file->flags != FS_FILE)
        return 0;

    file_system_t* fs = file_systems[file->device - 'a'];
    if (!fs)
        return 0;

    fs_data_t* data = &(fs->data);
    return fs->read_file(data, file, buffer, length);
}

size_t fsys_write_file(file_t* file, void* buffer, size_t length)
{
    if (!file || file->flags != FS_FILE)
        return 0;
    
    file_system_t* fs = file_systems[file->device - 'a'];
    if (!fs)
        return 0;

    fs_data_t* data = &(fs->data);
    return fs->write_file(data, file, buffer, length);
}

bool fsys_create_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname = (char*)filename;
    char device = 'a';
    if (filename[1] == ':')
    {
        device = filename[0];
        fname += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->create_file(data, fname);
}

bool fsys_delete_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname = (char*)filename;
    char device = 'a';
    if (filename[1] == ':')
    {
        device = filename[0];
        fname += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->delete_file(data, fname);
}

bool fsys_create_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dpath = (char*)dirpath;
    char device = 'a';
    if (dirpath[1] == ':')
    {
        device = dirpath[0];
        dpath += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->create_dir(data, dpath);
}

bool fsys_delete_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dpath = (char*)dirpath;
    char device = 'a';
    if (dirpath[1] == ':')
    {
        device = dirpath[0];
        dpath += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->delete_dir(data, dpath);
}

size_t fsys_get_position(file_t* file)
{
    if (!file || file->flags != FS_FILE)
        return 0;

    file_system_t* fs = file_systems[file->device - 'a'];
    if (!fs)
        return 0;

    fs_data_t* data = &(fs->data);
    return fs->get_position(data, file);
}

void fsys_set_position(file_t* file, size_t offset)
{
    if (!file || file->flags != FS_FILE)
        return;

    file_system_t* fs = file_systems[file->device - 'a'];
    if (!fs)
        return;

    fs_data_t* data = &(fs->data);
    return fs->set_position(data, file, offset);
}

bool fsys_list_dir(size_t* index, char* filename, const char* dirpath)
{
    if(!dirpath || !filename)
        return false;

    char* dpath = (char*)dirpath;
    char device = 'a';
    if (dirpath[1] == ':')
    {
        device = dirpath[0];
        dpath += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->list_dir(data, index, filename, dpath);
}

bool fsys_exists_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname = (char*)filename;
    char device = 'a';
    if (filename[1] == ':')
    {
        device = filename[0];
        fname += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->exists_file(data, fname);
}

bool fsys_exists_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dpath = (char*)dirpath;
    char device = 'a';
    if (dirpath[1] == ':')
    {
        device = dirpath[0];
        dpath += 2;
    }

    file_system_t* fs = file_systems[device - 'a'];
    if (!fs)
        return false;

    fs_data_t* data = &(fs->data);
    return fs->exists_dir(data, dpath);
}