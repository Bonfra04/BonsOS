#include <memory/phys_mem_manager.h>
#include <memory/memory_map.h>
#include <string.h>
#include <stdbool.h>

#define PMM_BLOCK_SIZE 0x1000
#define PMM_BLOCKS_PER_BYTE 8

static uint64_t max_blocks;
static uint64_t used_blocks;

static uint32_t* bitmap;

static inline void bitmap_set(size_t bit)
{
    bitmap[bit / 32] |= 1 << (bit % 32);
}

static inline void bitmap_unset(size_t bit)
{
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline bool bitmap_test(size_t bit)
{
    return bitmap[bit / 32] & (1 << (bit % 32));
}

static uint64_t bitmap_first_free()
{
    for(uint64_t i = 0; i < max_blocks / 32; i++)
        if(bitmap[i] != UINT32_MAX)
            for(uint8_t j = 0; j < 32; j++)
                if(!(bitmap[i] & (1 << j)))
                    return i * 32 + j;
    return -1;
}

static uint64_t bitmap_first_frees(size_t size)
{
    if(size == 0)
        return -1;
    if(size == 1)
        return bitmap_first_free();

    for(uint64_t i = 0; i < max_blocks / 32; i++)
        if(bitmap[i] != UINT32_MAX)
            for(uint8_t j = 0; j < 32; j++)
            {
                uint32_t bit = 1 << j;
                if(!(bitmap[i] & bit))
                {
                    uint32_t startingBit = i * 32 + bit;
                    size_t free = 0;
                    for(size_t count = 0; count <= size; count++)
                    {
                        if(!bitmap_test(startingBit + count))
                            free++;
                        else 
                            break;
                        if(free == size)
                            return i * 32 + j;
                    }
                }
            }
    return -1;
}

void pmm_init_region(uint64_t base_address, uint64_t region_length)
{
    uint64_t align = base_address / PMM_BLOCK_SIZE;
    uint64_t blocks = region_length / PMM_BLOCK_SIZE;

    while(blocks--)
    {
        bitmap_unset(align++);
        used_blocks++;
    }
}

void pmm_deinit_region(uint64_t base_address, uint64_t region_length)
{
    uint64_t align = base_address / PMM_BLOCK_SIZE;
    uint64_t blocks = region_length / PMM_BLOCK_SIZE;

    while(blocks--)
    {
        bitmap_set(align++);
        used_blocks++;
    }
}

void pmm_init(void* bitmap_addr)
{
    bitmap = bitmap_addr;

    max_blocks = memory_map.total_memory * 1024 / PMM_BLOCK_SIZE;
    used_blocks = 0;

    // By default all memory is in use
    memset(bitmap, 0xFF, max_blocks / PMM_BLOCKS_PER_BYTE);

    //first block is always set. This insures allocs cant be 0
    bitmap_set(0);

    for(uint32_t i = 0; i < memory_map.num_entries; i++)
        if(memory_map.entries[i].region_type == MEM_USABLE)
            pmm_init_region(memory_map.entries[i].base_address, memory_map.entries[i].region_length);

}

void* pmm_alloc_block()
{
    if(pmm_get_free_blocks() < 1)
        return 0; // out of memory

    uint64_t frame = bitmap_first_free();

    if(frame == -1)
        return 0; // out of memory

    bitmap_set(frame);

    used_blocks++;
    return (void*)(frame * PMM_BLOCK_SIZE);
}

void pmm_free_block(void* block)
{
    bitmap_unset((uint64_t)block / PMM_BLOCK_SIZE);
    used_blocks--;
}

void* pmm_alloc_blocks(size_t size)
{
    if(pmm_get_free_blocks() < size)
        return 0; // not enough memory

    uint64_t frame = bitmap_first_frees(size);

    if(frame == -1)
        return 0; // not enough memory

    for(size_t i = 0; i < size; i++)
        bitmap_set(frame + i);

    used_blocks += size;
    return (void*)(frame * PMM_BLOCK_SIZE);
}

void pmm_free_blocks(void* block, size_t size)
{
    uint64_t frame = (uint64_t)block / PMM_BLOCK_SIZE;
    for(size_t i = 0; i < size; i++)
        bitmap_unset(frame + i);

    used_blocks -= size;
}

inline uint64_t pmm_get_free_blocks()
{
    return max_blocks - used_blocks;
}

inline uint64_t pmm_get_block_size()
{
    return PMM_BLOCK_SIZE;
}