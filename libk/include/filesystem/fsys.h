#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Flags
#define FS_FILE       0b00
#define FS_DIRECTORY  0b01
#define FS_INVALID    0b10

#define FS_EOF -1

typedef bool (*fsys_interact_function)(size_t device, uint64_t lba, uint8_t count, void* address); 

typedef enum fsys_type
{
    FSYS_FAT16,
} fsys_type_t;

typedef struct file
{
    char name[32];
    uint32_t flags;
    uint32_t length;
    uint32_t first_cluster;
    uint64_t position;
    uint64_t sector;
    uint64_t cluster;
    char device_letter;
    bool eof;
    bool error;
} file_t;

typedef struct fs_data
{
    char name[8];
    fsys_interact_function reader;
    fsys_interact_function writer;
    size_t device;
    size_t offset;
    size_t size;
    uint8_t fs_specific[512];
} fs_data_t;

typedef struct file_system
{
    fs_data_t data;
 
    bool (*mount)(fs_data_t* fs);
    file_t (*open_file)(fs_data_t* fs, const char* filename);
    void (*close_file)(fs_data_t* fs, file_t* file);
    size_t (*read_file)(fs_data_t* fs, file_t* file, void* buffer, size_t length);
    size_t (*write_file)(fs_data_t* fs, file_t* file, void* buffer, size_t length);
    size_t (*get_position)(fs_data_t* fs, file_t* file);
    void (*set_position)(fs_data_t* fs, file_t* file, size_t offset);
    bool (*delete_file)(fs_data_t* fs, const char* filename);
    file_t (*create_file)(fs_data_t* fs, const char* filename);
    bool (*copy_file)(fs_data_t* fs, const char* oldpos, const char* newpos);
    bool (*move_file)(fs_data_t* fs, const char* oldpos, const char* newpos);
    bool (*exists_file)(fs_data_t* fs, const char* filename);

    bool (*delete_dir)(fs_data_t* fs, const char* dirname);
    file_t (*create_dir)(fs_data_t* fs, const char* dirname);

} file_system_t;

file_system_t fsys_generate(uint8_t type, size_t device, size_t offset, size_t size, fsys_interact_function disk_read, fsys_interact_function disk_write);

file_t fsys_open_file(const char* filename);
void fsys_close_file(file_t* file);
size_t fsys_read_file(file_t* file, void* buffer, size_t length);
size_t fsys_write_file(file_t* file, void* buffer, size_t length);
size_t fsys_get_position(file_t* file);
void fsys_set_position(file_t* file, size_t offset);
bool fsys_delete_file(const char* filename);
file_t fsys_create_file(const char* filename);
bool fsys_copy_file(const char* oldpos, const char* newpos);
bool fsys_move_file(const char* oldpos, const char* newpos);
bool fsys_exists_file(const char* filename);

void fsys_register_file_system(file_system_t* file_system, char device_letter);
void fsys_unregister_file_system(file_system_t* file_system);
file_system_t* fsys_get_filesystem(char letter);
