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

typedef struct file_system
{
    char name[8];
    bool (*mount)();
    file_t (*open_file)(const char* filename);
    void (*close_file)(file_t* file);
    size_t (*read_file)(file_t* file, void* buffer, size_t length);
    size_t (*write_file)(file_t* file, void* buffer, size_t length);
    size_t (*get_position)(file_t* file);
    bool (*set_position)(file_t* file, size_t offset);
} file_system_t;

file_t fsys_open_file(const char* filename);
void fsys_close_file(file_t* file);
size_t fsys_read_file(file_t* file, void* buffer, size_t length);
size_t fsys_write_file(file_t* file, void* buffer, size_t length);
size_t fsys_get_position(file_t* file);
bool fsys_set_position(file_t* file, size_t offset);

void fsys_register_file_system(file_system_t* file_system, char device_letter);
void fsys_unregister_file_system(file_system_t* file_system);
