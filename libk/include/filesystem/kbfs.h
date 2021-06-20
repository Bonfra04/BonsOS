#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

file_system_t kbfs_generate(size_t disk_id, size_t offset, size_t size);

bool kbfs_mount(fs_data_t* fs);

file_t kbfs_open_file(fs_data_t* fs, const char* filename, const char* mode);
bool kbfs_close_file(fs_data_t* fs, file_t* file);

size_t kbfs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t kbfs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);

bool kbfs_create_file(fs_data_t* fs, const char* filename);
bool kbfs_delete_file(fs_data_t* fs, const char* filename);

bool kbfs_create_dir(fs_data_t* fs, const char* dirpath);
bool kbfs_delete_dir(fs_data_t* fs, const char* dirpath);

size_t kbfs_get_position(fs_data_t* fs, file_t* file);
void kbfs_set_position(fs_data_t* fs, file_t* file, size_t position);

bool kbfs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath);

bool kbfs_exists_file(fs_data_t* fs, const char* filename);
bool kbfs_exists_dir(fs_data_t* fs, const char* dirpath);