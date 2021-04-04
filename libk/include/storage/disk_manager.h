#pragma once

#include <filesystem/fsys.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct partition
{
    uint8_t type;
    size_t lba_start;
    size_t length;
    file_system_t file_system;
} partition_t;

typedef struct disk
{
    partition_t partitions[4];
    size_t length;
} disk_t;

void disk_manager_init();
size_t disk_manager_disk_count();
disk_t* disk_manager_disks(); 

#ifdef __cplusplus
}
#endif