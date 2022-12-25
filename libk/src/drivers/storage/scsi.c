#include <drivers/storage/scsi.h>
#include <storage/storage.h>

#include <stdlib.h>
#include <string.h>

#include "scsi_types.h"

#define SCSI_TYPE_CD_DVD 0x05
#define SCSI_TYPE_DIRECT_ACCESS 0x00

static bool read_capacity(scsi_device_t* device)
{
    scsi_command_t command;
    memset(&command, 0, sizeof(scsi_command_t));
    command.write = false;
    command.packet_len = sizeof(scsi_command_read_capacity_t);
    scsi_command_read_capacity_t* packet = (scsi_command_read_capacity_t*)command.packet;

    packet->command = SCSI_COMMAND_READ_CAPACITY;

    scsi_read_capacity_t capacity;
    if(!device->driver.send_scsi_cmd(device->driver.data, &command, &capacity, sizeof(scsi_read_capacity_t)))
        return false;

    device->capacity = __builtin_bswap32(capacity.lba) + 1;
    device->sector_size = __builtin_bswap32(capacity.block_size);

    if(device->capacity == 0 && device->sector_size == 0)
        return false;

    return true;
}

static bool inquiry(scsi_device_t* device)
{
    scsi_command_t command;
    memset(&command, 0, sizeof(scsi_command_t));
    command.write = false;
    command.packet_len = sizeof(scsi_command_inquiry_t);
    scsi_command_inquiry_t* packet = (scsi_command_inquiry_t*)command.packet;

    packet->command = SCSI_COMMAND_INQUIRY;
    packet->evpd = 0;
    packet->allocation_length = __builtin_bswap16(sizeof(scsi_inquiry_t));

    scsi_inquiry_t inquiry;
    if(!device->driver.send_scsi_cmd(device->driver.data, &command, &inquiry, sizeof(scsi_inquiry_t)))
        return false;

    if(inquiry.peripheral_device_type != SCSI_TYPE_CD_DVD && inquiry.peripheral_device_type != SCSI_TYPE_DIRECT_ACCESS)
        return false; // TODO: what changes if it's not a cd?

    if(!read_capacity(device))
        return false;

    return true;
}

static bool scsi_read12(void* device, uint64_t lba, uint64_t nsectors, void* buffer)
{
    scsi_device_t* dev = (scsi_device_t*)device;

    scsi_command_t command;
    command.write = false;
    command.packet_len = sizeof(scsi_command_read12_t);
    scsi_command_read12_t* packet = (scsi_command_read12_t*)command.packet;

    packet->command = SCSI_COMMAND_READ12;
    packet->lba = __builtin_bswap32(lba);
    packet->length = __builtin_bswap32(nsectors);

    return dev->driver.send_scsi_cmd(dev->driver.data, &command, buffer, nsectors * dev->sector_size);
}

static bool scsi_read10(void* device, uint64_t lba, uint64_t nsectors, void* buffer)
{
    scsi_device_t* dev = (scsi_device_t*)device;

    scsi_command_t command;
    command.write = false;
    command.packet_len = sizeof(scsi_command_read10_t);
    scsi_command_read10_t* packet = (scsi_command_read10_t*)command.packet;

    packet->command = SCSI_COMMAND_READ10;
    packet->lba = __builtin_bswap32(lba);
    packet->length = __builtin_bswap16(nsectors);

    return dev->driver.send_scsi_cmd(dev->driver.data, &command, buffer, nsectors * dev->sector_size);
}

static bool scsi_write12(void* device, uint64_t lba, uint64_t nsectors, void* buffer)
{
    scsi_device_t* dev = (scsi_device_t*)device;

    scsi_command_t command;
    command.write = true;
    command.packet_len = sizeof(scsi_command_write12_t);
    scsi_command_write12_t* packet = (scsi_command_write12_t*)command.packet;

    packet->command = SCSI_COMMAND_WRITE12;
    packet->lba = __builtin_bswap32(lba);
    packet->length = __builtin_bswap32(nsectors);

    return dev->driver.send_scsi_cmd(dev->driver.data, &command, buffer, nsectors * dev->sector_size);
}

static bool scsi_write10(void* device, uint64_t lba, uint64_t nsectors, void* buffer)
{
    scsi_device_t* dev = (scsi_device_t*)device;

    scsi_command_t command;
    command.write = true;
    command.packet_len = sizeof(scsi_command_write10_t);
    scsi_command_write10_t* packet = (scsi_command_write10_t*)command.packet;

    packet->command = SCSI_COMMAND_WRITE10;
    packet->lba = __builtin_bswap32(lba);
    packet->length = __builtin_bswap16(nsectors);

    return dev->driver.send_scsi_cmd(dev->driver.data, &command, buffer, nsectors * dev->sector_size);
}

void scsi_register_device(scsi_driver_t driver)
{
    scsi_device_t* device = malloc(sizeof(scsi_device_t));
    device->driver = driver;

    if(!inquiry(device))
    {
        free(device);
        return;
    }

    storage_data_t storage_data;
    storage_data.sector_size = device->sector_size;
    storage_data.capacity = device->capacity;
    storage_data.data = device;
    storage_data.reader = device->driver.max_packet_size >= 12 ? scsi_read12 : scsi_read10;
    storage_data.writer = device->driver.max_packet_size >= 12 ? scsi_write12 : scsi_write10;
    storage_data.readonly = true;

    storage_register_device(storage_data);
}
