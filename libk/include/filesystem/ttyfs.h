#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

file_system_t ttyfs_generate(size_t disk_id, size_t offset, size_t size);

bool ttyfs_mount(fs_data_t* fs);

file_t ttyfs_open_file(fs_data_t* fs, const char* filename, const char* mode);
bool ttyfs_close_file(fs_data_t* fs, file_t* file);

size_t ttyfs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t ttyfs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);

bool ttyfs_create_file(fs_data_t* fs, const char* filename);
bool ttyfs_delete_file(fs_data_t* fs, const char* filename);

bool ttyfs_create_dir(fs_data_t* fs, const char* dirpath);
bool ttyfs_delete_dir(fs_data_t* fs, const char* dirpath);

size_t ttyfs_get_position(fs_data_t* fs, file_t* file);
void ttyfs_set_position(fs_data_t* fs, file_t* file, size_t position);

bool ttyfs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath);

bool ttyfs_exists_file(fs_data_t* fs, const char* filename);
bool ttyfs_exists_dir(fs_data_t* fs, const char* dirpath);