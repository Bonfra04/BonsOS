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
        disk->position = 0;
        disk->offset = 0;
        disk->length = ahci_get_capacity(i);
        disk->device = i;
        disk->reader = sata_read;
        disk->writer = sata_write;
        memset(&(disk->buffer), 0, 512);

        master_bootrecord_t* mbr = (master_bootrecord_t*)&(disk->buffer);
        if(!sata_read(i, 0, 1, (void*)mbr))
        {
            registered_disks--;
            continue;
        }

        for(int p = 0; p < 4; p++)
        {
            partition_t* partition = &(disk->partitions[p]);
            partition->type = mbr->partitions[p].type;

            switch (mbr->partitions[p].type)
            {
                case PTYPE_FAT16:
                {
                    partition->lba_start = mbr->partitions[p].start_lba;
                    partition->byte_start = mbr->partitions[p].start_lba * 512;
                    partition->length = mbr->partitions[p].sectors;
                    partition->file_system = fsys_generate(FSYS_FAT16, i, partition->byte_start, partition->length);
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

bool disk_manager_seek(size_t disk_id, size_t position)
{
    if(disk_id > registered_disks)
        return false;
    disk_t* disk = &(disks[disk_id]);

    if(disk->position < position && disk->position + 512 > position)
    {
        disk->offset = position - disk->position;
        return true;
    }

    if(!disk_manager_flush(disk_id))
        return false;

    size_t sector = position / 512;

    if(!disk->reader(disk->device, sector, 1, (void*)&(disk->buffer)))
        return false;

    disk->position = sector;
    disk->offset = position - sector * 512;
    return true;    
}

size_t disk_manager_read(size_t disk_id, size_t amount, void* buffer)
{
    if(disk_id > registered_disks)
        return 0;
    disk_t* disk = &(disks[disk_id]);

    size_t byte_read = 0;
    while (amount > 0)
    {
        size_t chunk_length = amount;
        if(chunk_length > 512 - disk->offset)
            chunk_length = 512 - disk->offset;

        memcpy((uint8_t*)buffer + byte_read, disk->buffer + disk->offset, chunk_length);
        disk->offset += chunk_length;
        byte_read += chunk_length;
        amount -= chunk_length;

        if(disk->offset == 512)
        {
            disk->offset = 0;
            disk_manager_flush(disk_id);
            disk->position++;
            if(!disk->reader(disk->device, disk->position, 1, (void*)&(disk->buffer)))
                return byte_read;
        }
    }

    return byte_read;
}

size_t disk_manager_write(size_t disk_id, size_t amount, void* buffer)
{
    if(disk_id > registered_disks)
        return 0;
    disk_t* disk = &(disks[disk_id]);

    size_t byte_written = 0;
    while (amount > 0)
    {
        size_t chunk_length = amount;
        if(chunk_length > 512 - disk->offset)
            chunk_length = 512 - disk->offset;

        memcpy(disk->buffer + disk->offset, (uint8_t*)buffer + byte_written, chunk_length);
        disk->offset += chunk_length;
        byte_written += chunk_length;
        amount -= chunk_length;

        if(disk->offset == 512)
        {
            disk->offset = 0;
            disk_manager_flush(disk_id);
            disk->position++;
            if(!disk->reader(disk->device, disk->position, 1, (void*)disk->buffer))
                return byte_written;
        }
    }

    return byte_written;
}

bool disk_manager_flush(size_t disk_id)
{
    if(disk_id > registered_disks)
        return false;
    disk_t* disk = &(disks[disk_id]);

    if(!disk->writer(disk->device, disk->position, 1, (void*)&(disk->buffer)))
        return false;

    return true;
}