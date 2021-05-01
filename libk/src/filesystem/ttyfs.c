#include <filesystem/ttyfs.h>
#include <string.h>
#include <device/tty.h>

file_system_t ttyfs_generate(size_t disk_id, size_t offset, size_t size)
{
    (void)disk_id; (void)offset; (void)size;
    file_system_t fs;
    memset(&fs, 0, sizeof(fs));
    return fs;
}

bool ttyfs_mount(fs_data_t* fs)
{
    (void)fs;
    return false;
}

file_t ttyfs_open_file(fs_data_t* fs, const char* filename, const char* mode)
{
    (void)fs; (void)filename; (void)mode;
    file_t invalid;
    invalid.error = true;
    invalid.flags = FS_INVALID;
    return invalid;
}

bool ttyfs_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return false;
}

size_t ttyfs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs; (void)file; (void)buffer; (void)length;
    return 0;
}

size_t ttyfs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs; (void)file;

    const char buff[length];
    memcpy((void*)buff, buffer, length);
    tty_print((const char*)&buff);

    return length;
}

bool ttyfs_create_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool ttyfs_delete_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool ttyfs_create_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

bool ttyfs_delete_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

size_t ttyfs_get_position(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return 0;
}

void ttyfs_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    (void)fs; (void)file; (void)position;
}

bool ttyfs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath)
{
    (void)fs; (void)index; (void)filename; (void)dirpath;
    return false;
}

bool ttyfs_exists_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool ttyfs_exists_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}