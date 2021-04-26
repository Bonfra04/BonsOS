#pragma once

#include <filesystem/fsys.h>
#include <stdbool.h>

bool free_cluster_chain(fs_data_t* fs, uint16_t cluster);
bool allocate_cluster(fs_data_t* fs, uint16_t* new_cluster, uint16_t previous_cluster);

bool get_next_cluster(fs_data_t* fs, uint16_t* next_cluster, uint16_t current_cluster);

void seek_to_data_region(fs_data_t* fs, size_t cluster, uint16_t offset);
size_t seek_to_fat_region(fs_data_t* fs, size_t cluster);
size_t seek_to_root_region(fs_data_t* fs, size_t entry_index);

file_t navigate_subdir(fs_data_t* fs, const char* dirpath);