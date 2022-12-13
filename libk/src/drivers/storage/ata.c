#include <drivers/storage/ata.h>
#include <drivers/pci.h>
#include <log.h>
#include <drivers/storage/ahci.h>
#include <storage/storage.h>

#include <stdlib.h>
#include <string.h>

#include "ata_types.h"

static bool identify(ata_device_t* dev)
{
    ata_ident_t ident;

    ata_command_t cmd;
    memset(&cmd, 0, sizeof(ata_command_t));
    cmd.command = ATA_CMD_IDENTIFY;

    if(!dev->send_ata_cmd(dev->data, &cmd, (void*)&ident, sizeof(ata_ident_t)))
        return false;

    dev->lba48 = (ident.command_set_2 & ATA_IDENT_CMD2_48BIT) && (ident.command_set_3 & ATA_IDENT_CMD3_48BIT);

    uint64_t sector_size = ident.sector_sz;
    if(sector_size & (1 << 14) && !(sector_size & (1 << 15)) // field contains valid info
    && sector_size & (1 << 12)) // logical sectors are larger than 512
        dev->sector_size = ident.words_per_sector;
    else
        dev->sector_size = 512;

    dev->capacity = ident.lba_capacity * dev->sector_size;

    return true;
}

static bool ata_rw(void* device, bool write, uint64_t lba, size_t nsectors, void* buffer)
{
    ata_device_t* dev = (ata_device_t*)device;

    if(nsectors == 0)
        return true;

    ata_command_t cmd;
    memset(&cmd, 0, sizeof(ata_command_t));
    cmd.command = dev->lba48 ? (write ? ATA_CMD_WRITEEXTDMA : ATA_CMD_READEXTDMA) : (write ? ATA_CMD_WRITEDMA : ATA_CMD_READDMA);
    cmd.lba = lba;
    cmd.n_sectors = nsectors;
    cmd.write = write;
    cmd.lba28 = !dev->lba48;

    return dev->send_ata_cmd(dev->data, &cmd, buffer, dev->sector_size * nsectors);
}

static bool ata_read(void* device, uint64_t lba, size_t nsectors, void* buffer)
{
    return ata_rw(device, false, lba, nsectors, buffer);
}

static bool ata_write(void* device, uint64_t lba, size_t nsectors, void* buffer)
{
    return ata_rw(device, true, lba, nsectors, buffer);
}

void ata_init()
{
    ahci_init();
}

void ata_register_device(ata_device_t ata_device)
{
    ata_device_t* dev = malloc(sizeof(ata_device_t));
    *dev = ata_device;

    if(!identify(dev))
        return;

    storage_data_t data;
    memset(&data, 0, sizeof(storage_data_t));
    data.capacity = dev->capacity;
    data.sector_size = dev->sector_size;
    data.data = dev;
    data.reader = ata_read;
    data.writer = ata_write;

    storage_register_device(data);
}
