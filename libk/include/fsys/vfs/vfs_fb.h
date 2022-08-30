#pragma once

#include <fsys/fsys.h>

/**
 * @brief initializes and mounts the fb vfs
 */
void vfs_fb_init();

file_system_t vfs_fb_instantiate(partition_descriptor_t partition);

file_t vfs_fb_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode);
bool vfs_fb_close_file(fs_data_t* fs, file_t* file);

size_t vfs_fb_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t vfs_fb_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length);

bool vfs_fb_create_file(fs_data_t* fs, const char* filename);
bool vfs_fb_delete_file(fs_data_t* fs, const char* filename);

bool vfs_fb_create_dir(fs_data_t* fs, const char* dirpath);
bool vfs_fb_delete_dir(fs_data_t* fs, const char* dirpath);

size_t vfs_fb_get_position(fs_data_t* fs, const file_t* file);
bool vfs_fb_set_position(fs_data_t* fs, file_t* file, size_t position);

bool vfs_fb_exists_file(fs_data_t* fs, const char* filename);
bool vfs_fb_exists_dir(fs_data_t* fs, const char* dirpath);

file_t vfs_fb_open_dir(fs_data_t* fs, const char* dirpath);
bool vfs_fb_list_dir(fs_data_t* fs, file_t* dir, direntry_t* entry);
bool vfs_fb_close_dir(fs_data_t* fs, file_t* dir);

bool vfs_fb_error(fs_data_t* fs, const file_t* file);
