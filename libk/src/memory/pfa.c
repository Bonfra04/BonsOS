#include <memory/pfa.h>
#include <memory/mmap.h>
#include <linker.h>
#include <alignment.h>
#include <panic.h>

#include <string.h>
#include <stdbool.h>

#define PAGES_PER_BYTE 8

static uint32_t* bitmap;
static size_t max_pages;
static size_t used_pages;

static inline uint64_t free_pages()
{
    return max_pages - used_pages;
}

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

static uint64_t bitmap_first_frees(size_t size)
{
    if(size == 0)
        return -1;

    for(uint64_t i = 0; i < max_pages / 32; i++)
        if(bitmap[i] != UINT32_MAX)
            for(uint64_t j = 0; j < 32; j++)
            {
                uint64_t bit = 1 << j;
                if(!(bitmap[i] & bit))
                {
                    uint64_t startingBit = i * 32 + j;
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

void pfa_init()
{
    extern symbol_t __kernel_end_addr;
    size_t kernel_end_aligned = ALIGN_4K_UP(__kernel_end_addr);
    const mmap_t* mmap = mmap_get();

    bitmap = (uint32_t*)kernel_end_aligned;
    max_pages = mmap->total_memory / PFA_PAGE_SIZE;
    size_t bitmap_size = ALIGN_4K_UP(max_pages / PAGES_PER_BYTE);

    // by default all memory is in use
    used_pages = max_pages;
    memset(bitmap, 0xFF, bitmap_size);

    // mark available memory as free
    for(size_t i = 0; i < mmap->num_entries; i++)
        if(mmap->entries[i].region_type == MEM_USABLE)
            pfa_init_region(mmap->entries[i].base_address, mmap->entries[i].region_length);

    // mark first mib, kernel and bitmap as used
    pfa_deinit_region(0, kernel_end_aligned + bitmap_size);
}

void* pfa_alloc(size_t count)
{
    if(free_pages() < count)
        kernel_panic("pfa_alloc: not enough free pages");

    uint64_t frame = bitmap_first_frees(count);

    if(frame == -1)
        kernel_panic("pfa_alloc: not enough free consecutive pages");

    for(size_t i = 0; i < count; i++)
        bitmap_set(frame + i);

    used_pages += count;
    return (void*)(frame * PFA_PAGE_SIZE);
}

void* pfa_calloc(size_t count)
{
    void* ptr = pfa_alloc(count);
    memset(ptr, 0, count * PFA_PAGE_SIZE);
    return ptr;
}

void pfa_free(void* ptr, size_t count)
{
    uint64_t frame = (uint64_t)ptr / PFA_PAGE_SIZE;

    for(size_t i = 0; i < count; i++)
        bitmap_unset(frame + i);

    used_pages -= count;
}

void pfa_init_region(void* base_address, size_t length)
{
    size_t base_aligned = ALIGN_4K_UP(base_address);
    size_t length_delta = base_aligned - (uint64_t)base_address;
    if(length_delta > length)
        return;
    size_t length_aligned = ALIGN_4K_DOWN(length - length_delta);
    if(length_aligned == 0)
        return;

    size_t pages = length_aligned / PFA_PAGE_SIZE;
    uint64_t page_index = base_aligned / PFA_PAGE_SIZE;

    while(pages--)
    {
        bitmap_unset(page_index++);
        used_pages--;
    }
}

void pfa_deinit_region(void* base_address, size_t length)
{
    size_t base_aligned = ALIGN_4K_DOWN(base_address);
    size_t length_aligned = ALIGN_4K_UP(length + (uint64_t)base_address - base_aligned);

    size_t pages = length_aligned / PFA_PAGE_SIZE;
    uint64_t page_index = base_aligned / PFA_PAGE_SIZE;

    while(pages--)
    {
        bitmap_set(page_index++);
        used_pages++;
    }
}
