#include <storage/storage.h>
#include <pci/ata/sata.h>

#include <string.h>
#include <assert.h>

#include <containers/vector.h>

#include "storage_types.h"

static vector_t storage_devices;

void storage_init()
{
    storage_devices = vector(storage_device_t);

    for(size_t i = 0; i < sata_ndisks(); i++)
    {
        storage_data_t data;
        memset(&data, 0, sizeof(storage_data_t));
        data.capacity = sata_get_capacity(i);
        data.internal_id = i;
        data.sector_size = sata_get_sector_size(i);
        data.reader = sata_read;
        data.writer = sata_write;

        storage_register_device(data);
    }
}

uint64_t storage_register_device(storage_data_t data)
{
    assert(data.sector_size <= MAX_BUFFER_LENGTH);

    storage_device_t device;
    memset(&device, 0, sizeof(storage_device_t));
    device.capacity = data.capacity;
    device.internal_id = data.internal_id;
    device.buff_len = data.sector_size;
    device.reader = data.reader;
    device.writer = data.writer;
    device.registered = true;
    device.lba_pos = 0;
    device.partitions = vector(partition_t);

    device.reader(device.internal_id, device.lba_pos, 1, device.buffer);
    const master_bootrecord_t* mbr = (master_bootrecord_t*)device.buffer;

    for(uint8_t i = 0; i < 4; i++)
    {
        partition_t partition;

        partition.type = mbr->partitions[i].type;
        partition.offset = mbr->partitions[i].start_lba * device.buff_len;
        partition.length = mbr->partitions[i].sectors * device.buff_len;

        if (partition.type != PART_TYPE_FREE)
            vector_push_back(&device.partitions, partition);
    }

    for (size_t i = 0; i < vector_size(&storage_devices); i++)
    {
        storage_device_t* dev = vector_at(&storage_devices, storage_device_t, i);
        if (!dev->registered)
        {
            *dev = device;
            return i;
        }
    }

    vector_push_back(&storage_devices, device);
    return vector_size(&storage_devices) - 1;
}

void storage_unregister_device(uint64_t id)
{
    if(id >= vector_size(&storage_devices))
        return;

    storage_device_t* device = vector_at(&storage_devices, storage_device_t, id);
    memset(device, -1, sizeof(storage_device_t));
    device->registered = false;
}

size_t storage_ndevices()
{
    size_t count = 0;
    for(size_t i = 0; i < vector_size(&storage_devices); i++)
    {
        storage_device_t* device = vector_at(&storage_devices, storage_device_t, i);
        if(device->registered)
            count++;
    }
    return count;
}

storage_descriptor_t storage_info(uint64_t id)
{
    storage_descriptor_t desc;
    memset(&desc, 0, sizeof(storage_descriptor_t));

    if(id >= vector_size(&storage_devices))
        return desc;

    storage_device_t* device = vector_at(&storage_devices, storage_device_t, id);
    desc.capacity = device->capacity;
    desc.partitions_count = vector_size(&device->partitions);
    desc.device_id = id;

    return desc;
}

partition_descriptor_t storage_get_partition(uint64_t id, uint64_t index)
{
    partition_descriptor_t desc;
    memset(&desc, 0, sizeof(partition_descriptor_t));

    if(id >= vector_size(&storage_devices))
        return desc;

    storage_device_t* device = vector_at(&storage_devices, storage_device_t, id);
    if(index >= vector_size(&device->partitions))
        return desc;

    partition_t* partition = vector_at(&device->partitions, partition_t, index);
    desc.device_id = id;
    desc.start = partition->offset;
    desc.length = partition->length;
    desc.type = partition->type;

    return desc;
}

static storage_device_t* get_device(size_t id)
{
    if(id >= vector_size(&storage_devices))
        return NULL;

    storage_device_t* device = vector_at(&storage_devices, storage_device_t, id);
    if(!device->registered)
        return NULL;

    return device;
}

bool storage_flush(size_t id)
{
    storage_device_t* device = get_device(id);
    if(!device)
        return false;

    return device->writer(device->internal_id, device->lba_pos, 1, device->buffer);
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

    return device->reader(device->internal_id, device->lba_pos, 1, device->buffer);
}

static bool flush_and_advance(storage_device_t* device)
{
    if(!device->writer(device->internal_id, device->lba_pos, 1, device->buffer))
        return false;

    device->buff_off = 0;
    device->lba_pos++;

    if(!device->reader(device->internal_id, device->lba_pos, 1, device->buffer))
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
        return false;

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
