#pragma once

#include <stdbool.h>
#include <filesystem/fsys.h>

file_t subdir_open_file(fs_data_t* fs, file_t* dir, const char* filename, const char* mode);
file_t subdir_open_dir(fs_data_t* fs, file_t* dir, const char* filename);

file_t subdir_create_file(fs_data_t* fs, file_t* dir, const char* filename);
file_t subdir_create_dir(fs_data_t* fs, file_t* dir, const char* dirpath);

bool subdir_delete_file(fs_data_t* fs, file_t* dir, const char* filename);
bool subdir_delete_dir(fs_data_t* fs, file_t* dir, const char* dirpath);

bool subdir_exists_file(fs_data_t* fs, file_t* dir, char* filename);
bool subdir_exists_dir(fs_data_t* fs, file_t* dir, char* dirpath);

file_t subdir_navigate(fs_data_t* fs, file_t* dir, const char* dirpath);