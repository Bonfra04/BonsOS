#include <fsys/vfs/vfs_tty.h>
#include <fsys/fsys.h>
#include <io/tty.h>
#include <io/keyboard/keyboard.h>

#include <stdlib.h>
#include <string.h>

void vfs_tty_init()
{
    fsys_mount_vfs(vfs_tty_instantiate, "tty");
}

file_system_t vfs_tty_instantiate(partition_descriptor_t partition)
{
    (void)partition;

    file_system_t fs;
    fs.open_file = vfs_tty_open_file;
    fs.close_file = vfs_tty_close_file;
    fs.read_file = vfs_tty_read_file;
    fs.write_file = vfs_tty_write_file;
    fs.create_file = vfs_tty_create_file;
    fs.delete_file = vfs_tty_delete_file;
    fs.create_dir = vfs_tty_create_dir;
    fs.delete_dir = vfs_tty_delete_dir;
    fs.get_position = vfs_tty_get_position;
    fs.set_position = vfs_tty_set_position;
    fs.exists_file = vfs_tty_exists_file;
    fs.exists_dir = vfs_tty_exists_dir;
    fs.open_dir = vfs_tty_open_dir;
    fs.list_dir = vfs_tty_list_dir;
    fs.close_dir = vfs_tty_close_dir;
    fs.error = vfs_tty_error;

    return fs;
}

typedef enum tty_mode
{
    TTY_RAW, TTY_COOKED
} tty_mode_t;

file_t vfs_tty_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode)
{
    (void)fs; (void)mode;

    tty_mode_t tty_mode;
    if(strcmp(filename, "/raw") == 0)
        tty_mode = TTY_RAW;
    else if(strcmp(filename, "/cooked") == 0)
        tty_mode = TTY_COOKED;
    else
        return INVALID_FILE;

    file_t f;
    *(tty_mode_t*)f.fs_data = tty_mode;
    return f;
}

bool vfs_tty_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;
    memset(file, 0, sizeof(file_t));
    return true;
}

size_t vfs_tty_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs;

    tty_mode_t mode = *(tty_mode_t*)file->fs_data;
    
    if(mode == TTY_COOKED)
        return tty_read(buffer, length);

    uint8_t* buf = buffer;
    for(size_t i = 0; i < length; i++)
        *buf++ = tty_read_raw();
    return length;
}

size_t vfs_tty_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length)
{
    (void)fs; (void)file;
    tty_print((char*)buffer);
    return length;
}

bool vfs_tty_create_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool vfs_tty_delete_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool vfs_tty_create_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

bool vfs_tty_delete_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

size_t vfs_tty_get_position(fs_data_t* fs, const file_t* file)
{
    (void)fs; (void)file;
    return 0;
}

bool vfs_tty_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    (void)fs; (void)file; (void)position;
    return false;
}

bool vfs_tty_exists_file(fs_data_t* fs, const char* filename)
{
    (void)fs;

    return strcmp(filename , "/raw") == 0 || strcmp(filename, "/cooked") == 0;
}

bool vfs_tty_exists_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

file_t vfs_tty_open_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return INVALID_FILE;
}

bool vfs_tty_list_dir(fs_data_t* fs, file_t* dir, direntry_t* entry)
{
    (void)fs; (void)dir; (void)entry;
    return false;
}

bool vfs_tty_close_dir(fs_data_t* fs, file_t* dir)
{
    (void)fs; (void)dir;
    return false;
}

bool vfs_tty_error(fs_data_t* fs, const file_t* file)
{
    (void)fs; (void)file;
    return false;
}
