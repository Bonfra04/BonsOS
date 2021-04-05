#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

file_system_t fat16_generate(size_t device, size_t offset, size_t size, fsys_interact_function disk_read, fsys_interact_function disk_write);

bool fat16_mount(fs_data_t* fs);
file_t fat16_open_file(fs_data_t* fs, const char* filename);
void fat16_close_file(fs_data_t* fs, file_t* file);
size_t fat16_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length);
size_t fat16_get_position(fs_data_t* fs, file_t* file);
void fat16_set_position(fs_data_t* fs, file_t* file, size_t position);
bool fat16_delete_file(fs_data_t* fs, const char* filename);
bool fat16_exists_file(fs_data_t* fs, const char* filename);
