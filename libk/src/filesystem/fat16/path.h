#pragma once

#include <stdbool.h>
#include <stddef.h>

bool to_short_filename(char* short_filename, const char* long_filename);
bool get_subdir(char* subdir_name, size_t* index, const char* path);
bool is_in_root(const char* path);

char* get_filename(const char* filename);