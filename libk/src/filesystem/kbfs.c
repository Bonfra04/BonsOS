#include <filesystem/kbfs.h>
#include <filesystem/ttyfs.h>
#include <string.h>

file_system_t kbfs_generate(size_t disk_id, size_t offset, size_t size)
{
    (void)disk_id; (void)offset; (void)size;
    file_system_t fs;
    memset(&fs, 0, sizeof(fs));
    return fs;
}

bool kbfs_mount(fs_data_t* fs)
{
    (void)fs;
    return false;
}

file_t kbfs_open_file(fs_data_t* fs, const char* filename, const char* mode)
{
    (void)fs; (void)filename; (void)mode;
    file_t invalid;
    invalid.error = true;
    invalid.flags = FS_INVALID;
    return invalid;
}

bool kbfs_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return false;
}

size_t kbfs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs; (void)file; (void)buffer; (void)length;

    uint8_t* buff = (uint8_t*)buffer;

    int ch = 0;
    while(length-- && ch != '\n')
    {
        ch = kb_getch();
        if(ch <= 0xFF)
        {
            switch (ch)
            {
            case '\b':
                buff--;
                ttyfs_write_file(0, 0, &ch, 1);
                length++;
                break;

            case '\n':
                ttyfs_write_file(0, 0, &ch, 1);
                break;
            
            default:
                *buff = (uint8_t)ch;
                ttyfs_write_file(0, 0, buff, 1);
                buff++;
            }
        }
    }

    return length;
}

size_t kbfs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs; (void)file; (void)buffer; (void)length;
    return 0;
}

bool kbfs_create_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool kbfs_delete_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool kbfs_create_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

bool kbfs_delete_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

size_t kbfs_get_position(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return 0;
}

void kbfs_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    (void)fs; (void)file; (void)position;
}

bool kbfs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath)
{
    (void)fs; (void)index; (void)filename; (void)dirpath;
    return false;
}

bool kbfs_exists_file(fs_data_t* fs, const char* filename)
{
    (void)fs; (void)filename;
    return false;
}

bool kbfs_exists_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}