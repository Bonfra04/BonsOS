#include <memory/mmap.h>

static mmap_t memory_map;

void mmap_init(uint32_t num_entries, void* map_address, uint64_t total_memory)
{
    memory_map.num_entries = num_entries;
    memory_map.entries = map_address;
    memory_map.total_memory = total_memory;
}

const mmap_t* mmap_get()
{
    return &memory_map;
}
