#include <memory/page_frame_allocator.h>
#include <memory/memory_map.h>
#include <string.h>
#include <stdbool.h>

#define PFA_PAGE_SIZE 0x1000
#define PFA_PAGES_PER_BYTE 8

static uint64_t max_pages;
static uint64_t used_pages;

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
    for(uint64_t i = 0; i < max_pages / 32; i++)
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

    for(uint64_t i = 0; i < max_pages / 32; i++)
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

void pfa_init_region(uint64_t base_address, uint64_t region_length)
{
    uint64_t align = base_address / PFA_PAGE_SIZE;
    uint64_t pages = region_length / PFA_PAGE_SIZE;

    while(pages--)
    {
        bitmap_unset(align++);
        used_pages++;
    }
}

void pfa_deinit_region(uint64_t base_address, uint64_t region_length)
{
    uint64_t align = base_address / PFA_PAGE_SIZE;
    uint64_t pages = region_length / PFA_PAGE_SIZE + 1;

    while(align <= max_pages && pages--)
    {
        bitmap_set(align++);
        used_pages++;
    }
}

void pfa_init(void* bitmap_addr)
{
    bitmap = bitmap_addr;

    max_pages = memory_map.total_memory * 1024 / PFA_PAGE_SIZE;
    used_pages = 0;

    // By default all memory is in use
    memset(bitmap, 0xFF, max_pages / PFA_PAGES_PER_BYTE);

    for(uint32_t i = 0; i < memory_map.num_entries; i++)
        if(memory_map.entries[i].region_type == MEM_USABLE)
            pfa_init_region(memory_map.entries[i].base_address, memory_map.entries[i].region_length);

    //first page is always set. This insures allocs cant be 0
    bitmap_set(0);
}

uint64_t pfa_get_bitmap_size()
{
    return memory_map.total_memory * 1024 / PFA_PAGE_SIZE / (sizeof(uint32_t) * 8);
}

void* pfa_alloc_page()
{
    if(pfa_get_free_pages() < 1)
        return 0; // out of memory

    uint64_t frame = bitmap_first_free();

    if(frame == -1)
        return 0; // out of memory

    bitmap_set(frame);

    used_pages++;
    return (void*)(frame * PFA_PAGE_SIZE);
}

void pfa_free_page(void* page)
{
    bitmap_unset((uint64_t)page / PFA_PAGE_SIZE);
    used_pages--;
}

void* pfa_alloc_pages(size_t size)
{
    if(pfa_get_free_pages() < size)
        return 0; // not enough memory

    uint64_t frame = bitmap_first_frees(size);

    if(frame == -1)
        return 0; // not enough memory

    for(size_t i = 0; i < size; i++)
        bitmap_set(frame + i);

    used_pages += size;
    return (void*)(frame * PFA_PAGE_SIZE);
}

void pfa_free_pages(void* page, size_t size)
{
    uint64_t frame = (uint64_t)page / PFA_PAGE_SIZE;
    for(size_t i = 0; i < size; i++)
        bitmap_unset(frame + i);

    used_pages -= size;
}

inline uint64_t pfa_get_free_pages()
{
    return max_pages - used_pages;
}

inline uint64_t pfa_page_size()
{
    return PFA_PAGE_SIZE;
}