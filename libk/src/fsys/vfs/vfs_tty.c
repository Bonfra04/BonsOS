#include <fsys/vfs/vfs_tty.h>
#include <fsys/fsys.h>
#include <io/tty.h>
#include <io/keyboard.h>

#include <stdlib.h>

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

file_t vfs_tty_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode)
{
    (void)fs; (void)mode;

    uint64_t id = atoull(filename + 1);
    // TODO: check valid tty
    
    file_t f;
    *(uint64_t*)f.fs_data = id;
    return f;
}

bool vfs_tty_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return false;
}

size_t vfs_tty_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs;

    uint64_t id = *(uint64_t*)file->fs_data;
    // TODO: use id

    return tty_read(buffer, length);
}

size_t vfs_tty_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length)
{
    (void)fs;

    uint64_t id = *(uint64_t*)file->fs_data;
    // TODO: use id

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
    (void)fs; (void)filename;
    return false;
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
