#pragma once

#include <fsys/fsys.h>

/**
 * @brief initializes and mounts the tty vfs
 */
void vfs_tty_init();

file_system_t vfs_tty_instantiate(partition_descriptor_t partition);

file_t vfs_tty_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode);
bool vfs_tty_close_file(fs_data_t* fs, file_t* file);

size_t vfs_tty_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t vfs_tty_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length);

bool vfs_tty_create_file(fs_data_t* fs, const char* filename);
bool vfs_tty_delete_file(fs_data_t* fs, const char* filename);

bool vfs_tty_create_dir(fs_data_t* fs, const char* dirpath);
bool vfs_tty_delete_dir(fs_data_t* fs, const char* dirpath);

size_t vfs_tty_get_position(fs_data_t* fs, const file_t* file);
bool vfs_tty_set_position(fs_data_t* fs, file_t* file, size_t position);

bool vfs_tty_exists_file(fs_data_t* fs, const char* filename);
bool vfs_tty_exists_dir(fs_data_t* fs, const char* dirpath);

file_t vfs_tty_open_dir(fs_data_t* fs, const char* dirpath);
bool vfs_tty_list_dir(fs_data_t* fs, file_t* dir, direntry_t* entry);
bool vfs_tty_close_dir(fs_data_t* fs, file_t* dir);
