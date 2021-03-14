#include <filesystem/fsys.h>

#define DEVICE_MAX 26

static file_system_t* file_systems[DEVICE_MAX];

file_t fsys_open_file(const char* filename)
{
    if(filename)
    {
        char* fname = (char*)filename;

        //default to device 'a'
        char device = 'a';    

        if (filename[1] == ':')
        {
            device = filename[0];
            fname += 2;
        }

        //call filesystem
        if (file_systems[device - 'a'])
        {
            //set volume specific information and return file
            file_t file = file_systems[device - 'a']->open_file(fname);
            file.device_letter = device;
            return file;
        }
    }

    file_t file;
    file.flags = FS_INVALID;
    return file;
}

void fsys_close_file(file_t* file)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            file_systems[file->device_letter - 'a']->close_file(file);
}

size_t fsys_read_file(file_t* file, void* buffer, size_t length)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->read_file(file, buffer, length);
    return 0;
}

size_t fsys_write_file(file_t* file, void* buffer, size_t length)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->write_file(file, buffer, length);
    return 0;
}

size_t fsys_get_position(file_t* file)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->get_position(file);
    return 0;
}

void fsys_set_position(file_t* file, size_t offset)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            file_systems[file->device_letter - 'a']->set_position(file, offset);
}

void fsys_register_file_system(file_system_t* file_system, char device_letter)
{
    static uint8_t registered_devices = 0;

    if (registered_devices < DEVICE_MAX)
        if (file_system) 
        {
            file_systems[device_letter - 'a'] = file_system;
            registered_devices++;
        }
}

void fsys_unregister_file_system(file_system_t* file_system)
{
    for (uint8_t i = 0; i < DEVICE_MAX; i++)
        if (file_systems[i] == file_system)
            file_systems[i] = 0;
}