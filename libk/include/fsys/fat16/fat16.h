#pragma once

#include <fsys/fsys.h>

file_system_t fat16_instantiate(partition_descriptor_t partition);

file_t fat16_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode);
bool fat16_close_file(fs_data_t* fs, file_t* file);

size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t fat16_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length);

bool fat16_create_file(fs_data_t* fs, const char* filename);
bool fat16_delete_file(fs_data_t* fs, const char* filename);

bool fat16_create_dir(fs_data_t* fs, const char* dirpath);
bool fat16_delete_dir(fs_data_t* fs, const char* dirpath);

size_t fat16_get_position(fs_data_t* fs, const file_t* file);
bool fat16_set_position(fs_data_t* fs, file_t* file, size_t position);

bool fat16_exists_file(fs_data_t* fs, const char* filename);
bool fat16_exists_dir(fs_data_t* fs, const char* dirpath);

file_t fat16_open_dir(fs_data_t* fs, const char* dirpath);
bool fat16_list_dir(fs_data_t* fs, file_t* dir, direntry_t* entry);
bool fat16_close_dir(fs_data_t* fs, file_t* dir);

bool fat16_error(fs_data_t* fs, const file_t* file);
