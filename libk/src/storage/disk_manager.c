#include <storage/disk_manager.h>
#include <stdint.h>
#include <device/ata/ahci.h>
#include <string.h>
#include "disk_manager_types.h"

static disk_t disks[MAX_DISKS];
static size_t registered_disks;
static uint8_t current_letter;

void disk_manager_init()
{
    registered_disks = 0;
    current_letter = 'a';

    for(size_t i = 0; i < ahci_devices(); i++)
    {
        disk_t* disk = &(disks[registered_disks++]);
        disk->length = ahci_get_capacity(i);

        master_bootrecord_t mbr;
        if(!sata_read(i, 0, 1, (void*)&mbr))
            continue;

        for(int p = 0; p < 4; p++)
        {
            partition_t* partition = &(disk->partitions[p]);
            partition->type = mbr.partitions[p].type;

            switch (mbr.partitions[p].type)
            {
                case PTYPE_FAT16:
                {
                    partition->lba_start = mbr.partitions[p].start_lba;
                    partition->length = mbr.partitions[p].sectors;
                    partition->file_system = fsys_generate(FSYS_FAT16, i, partition->lba_start, partition->length, sata_read, sata_write);
                    fsys_register_file_system(&(partition->file_system), current_letter++);
                    break;
                }

                case PTYPE_FREE:
                default:
                {
                    partition->lba_start = 0;
                    partition->length = 0;
                    break;
                }
            }
        }
    }
}

size_t disk_manager_disk_count()
{
    return registered_disks;
}

disk_t* disk_manager_disks()
{
    return (disk_t*)&disks;
}