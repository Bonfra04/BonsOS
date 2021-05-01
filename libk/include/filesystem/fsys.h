#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FS_INVALID     0b00
#define FS_FILE        0b01
#define FS_DIRECTORY   0b10

#define FS_EOF -1

typedef enum fsys_type
{
    FSYS_FAT16,
} fsys_type_t;

typedef struct file
{
    char name[30];
    size_t length;
    
    uint8_t flags;
    char mode;
    bool update;
    char device;
    bool error;
    bool eof;
    uint8_t data[512];
} file_t;

typedef struct fs_data
{
    char name[8];
    size_t disk_id;
    size_t offset;
    size_t size;
    uint8_t fs_specific[512];
} fs_data_t;

typedef struct file_system
{
    fs_data_t data;
 
    bool (*mount)(fs_data_t* fs);

    file_t (*open_file)(fs_data_t* fs, const char* filename, const char* mode);
    bool (*close_file)(fs_data_t* fs, file_t* file);

    size_t (*read_file)(fs_data_t* fs, file_t* file, void* buffer, size_t length);
    size_t (*write_file)(fs_data_t* fs, file_t* file, void* buffer, size_t length);

    bool (*create_file)(fs_data_t* fs, const char* filename);
    bool (*delete_file)(fs_data_t* fs, const char* filename);

    bool (*create_dir)(fs_data_t* fs, const char* dirpath);
    bool (*delete_dir)(fs_data_t* fs, const char* dirpath);

    size_t (*get_position)(fs_data_t* fs, file_t* file);
    void (*set_position)(fs_data_t* fs, file_t* file, size_t offset);

    bool (*list_dir)(fs_data_t* fs, size_t* index, char* filename, const char* dirpath);
    
    bool (*exists_file)(fs_data_t* fs, const char* filename);
    bool (*exists_dir)(fs_data_t* fs, const char* dirpath);
} file_system_t;

file_system_t fsys_generate(uint8_t type, size_t disk_id, size_t offset, size_t size);

void fsys_register_file_system(file_system_t* file_system, char device_letter);
void fsys_unregister_file_system(file_system_t* file_system);

file_t fsys_open_file(const char* filename, const char* mode);
bool fsys_close_file(file_t* file);

size_t fsys_read_file(file_t* file, void* buffer, size_t length);
size_t fsys_write_file(file_t* file, void* buffer, size_t length);

bool fsys_create_file(const char* filename);
bool fsys_delete_file(const char* filename);

bool fsys_create_dir(const char* dirpath);
bool fsys_delete_dir(const char* dirpath);

size_t fsys_get_position(file_t* file);
void fsys_set_position(file_t* file, size_t offset);

bool fsys_list_dir(size_t* index, char* filename, const char* dirpath);

bool fsys_exists_file(const char* filename);
bool fsys_exists_dir(const char* dirpath);
