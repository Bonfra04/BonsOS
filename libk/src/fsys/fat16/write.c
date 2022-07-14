#include <math.h>

#include "fat16_utils.h"

size_t write_entry(const fat16_data_t* data, fat16_entry_t* entry, const void* buffer, size_t length)
{
    size_t bytes_written = 0;

    while(length > 0)
    {
        // prepare next cluster
        if(entry->cluster_offset == data->bytes_per_cluster || entry->cluster == 0)
        {
            entry->cluster_offset = 0;

            uint64_t new_cluster = 0;
            bool state = !get_next_cluster(data, entry->cluster, &new_cluster);
            if(new_cluster >= 0xFFF8 && new_cluster <= 0xFFFF)
                state = allocate_cluster(data, entry->cluster, &new_cluster);
            if(!state)
            {
                entry->error = true;
                break;
            }

            entry->cluster = new_cluster;
            
            if(entry->first_cluster == 0)
            {
                entry->first_cluster = new_cluster;
                
                if(storage_seek_write(data->storage_id, data->offset + entry->entry_addr + offsetof(dir_entry_t, first_cluster), sizeof(uint16_t), &new_cluster) != sizeof(uint16_t))
                    return false;
            }
        }

        // calculate to write length
        size_t chunk_length = ullmin(length, data->bytes_per_cluster - entry->cluster_offset);

        // write chunk
        uint64_t addr = (entry->cluster - FIRST_CLUSTER_OFFSET) * data->bytes_per_cluster + data->data_start + entry->cluster_offset;
        if(storage_seek_write(data->storage_id, data->offset + addr, chunk_length, buffer) != chunk_length)
        {
            entry->error = true;
            break;
        }

        // adjust entry
        entry->cluster_offset += chunk_length;
        entry->advance += chunk_length;
        bytes_written += chunk_length;
        length -= chunk_length;
        buffer = (uint8_t*)buffer + chunk_length;
    }

    // update file size in entry
    if(entry->type == FAT16_FILE)
    {
        entry->length += bytes_written;
        if(storage_seek_write(data->storage_id, data->offset + entry->entry_addr + offsetof(dir_entry_t, file_size), sizeof(uint32_t), &entry->length) != sizeof(uint32_t))
            entry->error = true;
    }

    return bytes_written;
}
