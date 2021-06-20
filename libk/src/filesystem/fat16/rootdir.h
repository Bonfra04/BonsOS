#pragma once

#include <stdbool.h>
#include <filesystem/fsys.h>

file_t rootdir_open_file(fs_data_t* fs, const char* filename, const char* mode);
file_t rootdir_open_dir(fs_data_t* fs, const char* filename);

bool rootdir_create_file(fs_data_t* fs, const char* filename);
bool rootdir_create_dir(fs_data_t* fs, const char* dirpath);

bool rootdir_delete_file(fs_data_t* fs, const char* filename);
bool rootdir_delete_dir(fs_data_t* fs, const char* dirpath);

bool rootdir_exists_file(fs_data_t* fs, const char* filename);
bool rootdir_exists_dir(fs_data_t* fs, const char* dirpath);