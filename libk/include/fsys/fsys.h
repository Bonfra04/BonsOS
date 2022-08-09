#pragma once

#include <storage/storage.h>

#define FNAME_MAX_LEN 255

typedef struct file_system file_system_t;

typedef struct file
{
    file_system_t* fsys;
    uint8_t fs_data[512];
} file_t;

typedef struct direntry
{
    uint8_t flags;
    char name[FNAME_MAX_LEN + 1];
    uint8_t fs_data[512];
} direntry_t;

#define FSYS_FLG_DIR       0x01
#define FSYS_FLG_FILE      0x02
#define FSYS_FLG_READ_ONLY 0x04

#define INVALID_FILE (file_t){ .fsys = NULL, .fs_data = { 0 } }

typedef struct fs_data
{
    size_t disk_id;
    size_t offset;
    size_t length;
    uint8_t fs_specific[512];
} fs_data_t;

typedef enum fsys_file_mode
{
    FSYS_READ, FSYS_WRITE, FSYS_APPEND,
} fsys_file_mode_t;

typedef struct file_system
{
    uint8_t type;
    fs_data_t data;

    file_t (*open_file)(fs_data_t* fs, const char* filename, fsys_file_mode_t mode);
    bool (*close_file)(fs_data_t* fs, file_t* file);

    size_t (*read_file)(fs_data_t* fs, file_t* file, void* buffer, size_t length);
    size_t (*write_file)(fs_data_t* fs, file_t* file, const void* buffer, size_t length);

    bool (*create_file)(fs_data_t* fs, const char* filename);
    bool (*delete_file)(fs_data_t* fs, const char* filename);

    bool (*create_dir)(fs_data_t* fs, const char* dirpath);
    bool (*delete_dir)(fs_data_t* fs, const char* dirpath);

    size_t (*get_position)(fs_data_t* fs, const file_t* file);
    bool (*set_position)(fs_data_t* fs, file_t* file, size_t offset);

    bool (*exists_file)(fs_data_t* fs, const char* filename);
    bool (*exists_dir)(fs_data_t* fs, const char* dirpath);

    file_t (*open_dir)(fs_data_t* fs, const char* dirpath);
    bool (*list_dir)(fs_data_t* fs, file_t* dir, direntry_t* entry);
    bool (*close_dir)(fs_data_t* fs, file_t* dir);

    bool (*error)(fs_data_t* fs, const file_t* file);
} file_system_t;

typedef file_system_t (*fsys_instantiate_t)(partition_descriptor_t partition);

/**
 * @brief initializes the file system abstraction
 */
void fsys_init();

/**
 * @brief fetches all connected devices and registers all known file systems
 * @note should only be called once
 */
void fsys_auto_mount();

/**
 * @brief registers a file system constructor
 * @param[in] instantiate_function the function to instantiate the file system
 * @param[in] type the type of the file system
 */
void fsys_register(fsys_instantiate_t instantiate_function, uint8_t type);

/**
 * @brief mounts a file system for the given partition with the given name
 * @param[in] partition the partition descriptor
 * @param[in] name the name of the file system
 * @return true if the file system was mounted, false otherwise
 */
bool fsys_mount(partition_descriptor_t partition, const char* name);

/**
 * @brief mounts a virtual file system with the given name
 * @param[in] instantiate_function the function to instantiate the file system
 * @param[in] name the name of the file system
 * @return true if the file system was mounted, false otherwise
 */
bool fsys_mount_vfs(fsys_instantiate_t instantiate_function, const char* name);

/**
 * @brief unmounts the file system with the given name
 * @param[in] name the name of the file system
 */
void fsys_unmount(const char* name);

/**
 * @brief returns the file system type for the given name
 * @param[in] name the name of the file system
 * @return the file system type
 */
uint8_t fsys_type(const char* name);

file_t fsys_open_file(const char* filename, fsys_file_mode_t mode);
bool fsys_close_file(file_t* file);

size_t fsys_read_file(file_t* file, void* buffer, size_t length);
size_t fsys_write_file(file_t* file, void* buffer, size_t length);

bool fsys_create_file(const char* filename);
bool fsys_delete_file(const char* filename);

bool fsys_create_dir(const char* dirpath);
bool fsys_delete_dir(const char* dirpath);

size_t fsys_get_position(file_t* file);
bool fsys_set_position(file_t* file, size_t offset);

bool fsys_exists_file(const char* filename);
bool fsys_exists_dir(const char* dirpath);

file_t fsys_open_dir(const char* dirpath);
bool fsys_list_dir(file_t* dir, direntry_t* entry);
bool fsys_close_dir(file_t* dir);

bool fsys_error(file_t* file);
