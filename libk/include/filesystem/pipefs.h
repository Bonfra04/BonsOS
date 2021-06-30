#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

file_system_t pipefs_generate(size_t disk_id, size_t offset, size_t size);

bool pipefs_mount(fs_data_t* fs);

file_t pipefs_open_file(fs_data_t* fs, const char* filename, const char* mode);
bool pipefs_close_file(fs_data_t* fs, file_t* file);

size_t pipefs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t pipefs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);

bool pipefs_create_file(fs_data_t* fs, const char* filename);
bool pipefs_delete_file(fs_data_t* fs, const char* filename);

bool pipefs_create_dir(fs_data_t* fs, const char* dirpath);
bool pipefs_delete_dir(fs_data_t* fs, const char* dirpath);

size_t pipefs_get_position(fs_data_t* fs, file_t* file);
void pipefs_set_position(fs_data_t* fs, file_t* file, size_t position);

bool pipefs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath);

bool pipefs_exists_file(fs_data_t* fs, const char* filename);
bool pipefs_exists_dir(fs_data_t* fs, const char* dirpath);