#include <memory/memory_map.h>

memory_map_t memory_map;

void memory_map_init(uint32_t num_entries, void* map_address, uint64_t total_memory)
{
    memory_map.num_entries = num_entries;
    memory_map.entries = map_address;
    memory_map.total_memory = total_memory;
}