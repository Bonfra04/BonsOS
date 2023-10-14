#include <storage/storage.h>
#include <drivers/storage/ata.h>
#include <memory/pfa.h>

#include <string.h>
#include <stdlib.h>

#include <containers/darray.h>

#include "storage_types.h"

static storage_device_t* storage_devices;

void storage_init()
{
    storage_devices = darray(storage_device_t, 0);
}

uint64_t storage_register_device(storage_data_t data)
{
    storage_device_t device;
    memset(&device, 0, sizeof(storage_device_t));
    device.capacity = data.capacity;
    device.data = data.data;
    device.buff_len = data.sector_size;
    device.reader = data.reader;
    device.writer = data.writer;
    device.registered = true;
    device.lba_pos = 0;
    device.partitions = darray(partition_t, 0);
    // TODO: calculate proper size
    device.buffer = pfa_alloc(1);
    device.readonly = data.readonly;
    
    device.reader(device.data, device.lba_pos, 1, device.buffer);
    const master_bootrecord_t* mbr = (master_bootrecord_t*)device.buffer;

    for(uint8_t i = 0; i < 4; i++)
    {
        partition_t partition;

        partition.type = mbr->partitions[i].type;
        partition.offset = mbr->partitions[i].start_lba * device.buff_len;
        partition.length = mbr->partitions[i].sectors * device.buff_len;

        if (partition.type != PART_TYPE_FREE)
            darray_append(device.partitions, partition);
    }

    for (size_t i = 0; i < darray_length(storage_devices); i++)
    {
        if (!storage_devices[i].registered)
        {
            storage_devices[i] = device;
            return i;
        }
    }
    
    darray_append(storage_devices, device);
    return darray_length(storage_devices) - 1;
}

void storage_unregister_device(uint64_t id)
{
    if(id >= darray_length(storage_devices))
        return;

    free(storage_devices[id].buffer);
    memset(&storage_devices[id], -1, sizeof(storage_device_t));
    storage_devices[id].registered = false;
}

size_t storage_ndevices()
{
    size_t count = 0;
    for(size_t i = 0; i < darray_length(storage_devices); i++)
    {
        if(storage_devices[i].registered)
            count++;
    }
    return count;
}

storage_descriptor_t storage_info(uint64_t id)
{
    storage_descriptor_t desc;
    memset(&desc, 0, sizeof(storage_descriptor_t));

    if(id >= darray_length(storage_devices))
        return desc;

    desc.capacity = storage_devices[id].capacity;
    desc.partitions_count = darray_length(storage_devices[id].partitions);
    desc.device_id = id;

    return desc;
}

partition_descriptor_t storage_get_partition(uint64_t id, uint64_t index)
{
    partition_descriptor_t desc;
    memset(&desc, 0, sizeof(partition_descriptor_t));

    if(id >= darray_length(storage_devices))
        return desc;

    if(index >= darray_length(storage_devices[id].partitions))
        return desc;

    partition_t* partition = &storage_devices[id].partitions[index];
    desc.device_id = id;
    desc.start = partition->offset;
    desc.length = partition->length;
    desc.type = partition->type;

    return desc;
}

static storage_device_t* get_device(size_t id)
{
    if(id >= darray_length(storage_devices))
        return NULL;

    if(!storage_devices[id].registered)
        return NULL;

    return &storage_devices[id];
}

bool storage_flush(size_t id)
{
    storage_device_t* device = get_device(id);
    if(!device)
        return false;
    if(device->readonly)
        return true;

    return device->writer(device->data, device->lba_pos, 1, device->buffer);
}

bool storage_seek(size_t id, size_t position)
{
    storage_device_t* device = get_device(id);
    if(!device)
        return false;

    if(!storage_flush(id))
        return false;

    device->lba_pos = position / device->buff_len;
    device->buff_off = position % device->buff_len;

    return device->reader(device->data, device->lba_pos, 1, device->buffer);
}

static bool flush_and_advance(storage_device_t* device)
{
    if(!device->readonly && !device->writer(device->data, device->lba_pos, 1, device->buffer))
        return false;

    device->buff_off = 0;
    device->lba_pos++;

    if(!device->reader(device->data, device->lba_pos, 1, device->buffer))
        return false;

    return true;
}

uint64_t storage_read(size_t id, size_t amount, const void* address)
{
    storage_device_t* device = get_device(id);
    if(!device)
        return false;

    uint64_t byte_read = 0;
    while(amount > 0)
    {
        size_t chunk_length = amount;
        if(chunk_length > device->buff_len - device->buff_off)
            chunk_length = device->buff_len - device->buff_off;

        memcpy((uint8_t*)address + byte_read, device->buffer + device->buff_off, chunk_length);

        device->buff_off += chunk_length;
        byte_read += chunk_length;
        amount -= chunk_length;

        if(device->buff_off == device->buff_len && !flush_and_advance(device))
            return byte_read;
    }

    return byte_read;
}

uint64_t storage_write(size_t id, size_t amount, const void* address)
{
    storage_device_t* device = get_device(id);
    if(!device)
        return 0;
    if(device->readonly)
        return 0;

    uint64_t byte_written = 0;
    while(amount > 0)
    {
        size_t chunk_length = amount;
        if(chunk_length > device->buff_len - device->buff_off)
            chunk_length = device->buff_len - device->buff_off;

        memcpy(device->buffer + device->buff_off, (uint8_t*)address + byte_written, chunk_length);

        device->buff_off += chunk_length;
        byte_written += chunk_length;
        amount -= chunk_length;

        if(device->buff_off == device->buff_len && !flush_and_advance(device))
            return byte_written;
    }

    return byte_written;
}

uint64_t storage_seek_read(size_t id, size_t position, size_t amount, const void* address)
{
    if(!storage_seek(id, position))
        return 0;
    return storage_read(id, amount, address);
}

uint64_t storage_seek_write(size_t id, size_t position, size_t amount, const void* address)
{
    if(!storage_seek(id, position))
        return 0;
    return storage_write(id, amount, address);
}
