#include <filesystem/fsys.h>
#include <device/ata/ahci.h>
#include <filesystem/fat16.h>

#define DEVICE_MAX 26

static file_system_t* file_systems[DEVICE_MAX];

file_system_t fsys_generate(uint8_t type, size_t device, size_t offset, size_t size, fsys_interact_function disk_read, fsys_interact_function disk_write)
{
    switch (type)
    {
    case FSYS_FAT16:
        return fat16_generate(device, offset, size, disk_read, disk_write);
    }
    
    file_system_t invalid;
    return invalid;
}

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
            file_t file = file_systems[device - 'a']->open_file(&(file_systems[device - 'a']->data), fname);
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
            file_systems[file->device_letter - 'a']->close_file(&(file_systems[file->device_letter - 'a']->data), file);
}

size_t fsys_read_file(file_t* file, void* buffer, size_t length)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->read_file(&(file_systems[file->device_letter - 'a']->data), file, buffer, length);
    return 0;
}

size_t fsys_write_file(file_t* file, void* buffer, size_t length)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->write_file(&(file_systems[file->device_letter - 'a']->data), file, buffer, length);
    return 0;
}

size_t fsys_get_position(file_t* file)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            return file_systems[file->device_letter - 'a']->get_position(&(file_systems[file->device_letter - 'a']->data), file);
    return 0;
}

void fsys_set_position(file_t* file, size_t offset)
{
    if (file && file->flags != FS_INVALID)
        if (file_systems[file->device_letter - 'a'])
            file_systems[file->device_letter - 'a']->set_position(&(file_systems[file->device_letter - 'a']->data), file, offset);
}

bool fsys_delete_file(const char* filename)
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
            return file_systems[device - 'a']->delete_file(&(file_systems[device - 'a']->data), fname);
    }

    return false;
}

file_t fsys_create_file(const char* filename)
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
            return file_systems[device - 'a']->create_file(&(file_systems[device - 'a']->data), fname);
    }

    file_t file;
    file.flags = FS_INVALID;
    return file;
}

bool fsys_copy_file(const char* oldpos, const char* newpos)
{
    if(!oldpos || !newpos)
        return;

    char* oldname = (char*)oldpos;
    char olddevice = 'a';
    if (oldpos[1] == ':')
    {
        olddevice = oldpos[0];
        oldname += 2;
    }

    char* newname = (char*)newpos;
    char newdevice = 'a';
    if (oldpos[1] == ':')
    {
        newdevice = newpos[0];
        newname += 2;
    }

    //call filesystem
    //if (file_systems[olddevice - 'a'] && file_systems[newdevice - 'a'])
        //return file_systems[olddevice - 'a']->copy_file(fname);
    
    return false;
}

bool fsys_move_file(const char* oldpos, const char* newpos);
bool fsys_exists_file(const char* filename);

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