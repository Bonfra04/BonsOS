#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

file_system_t fat16_generate(size_t disk_id, size_t offset, size_t size);

bool fat16_mount(fs_data_t* fs);

file_t fat16_open_file(fs_data_t* fs, const char* filename, const char* mode);
bool fat16_close_file(fs_data_t* fs, file_t* file);

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t fat16_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);

bool fat16_create_file(fs_data_t* fs, const char* filename);
bool fat16_delete_file(fs_data_t* fs, const char* filename);

bool fat16_create_dir(fs_data_t* fs, const char* dirpath);
bool fat16_delete_dir(fs_data_t* fs, const char* dirpath);

size_t fat16_get_position(fs_data_t* fs, file_t* file);
void fat16_set_position(fs_data_t* fs, file_t* file, size_t position);

bool fat16_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath);

bool fat16_exists_file(fs_data_t* fs, const char* filename);
bool fat16_exists_dir(fs_data_t* fs, const char* dirpath);