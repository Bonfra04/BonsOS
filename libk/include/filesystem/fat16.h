#pragma once

#include <stdint.h>
#include <filesystem/fsys.h>

void fat16_init(char device_letter, size_t device_id, size_t offset, fsys_interact_function read_function, fsys_interact_function write_function);
bool fat16_mount();

file_t fat16_open_file(const char* filename);
void fat16_close_file(file_t* file);
size_t fat16_read_file(file_t* file, void* buffer, size_t length);
size_t fat16_write_file(file_t* file, void* buffer, size_t length);
size_t fat16_get_position(file_t* file);
void fat16_set_position(file_t* file, size_t position);
