#include <memory/heap.h>
#include <alignment.h>
#include <atomic/mutex.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct heap_region heap_region_t;
typedef struct heap_region
{
    uint64_t length;
    heap_region_t* next_region;
    heap_region_t* previous_region;
    bool free;
    uint8_t chunk_id;
} heap_region_t;

static void* base_address;
static uint64_t size;
static heap_region_t* first_region;
static mutex_t heap_mutex;

#ifdef KERNEL_BUILD
#include <memory/pfa.h>
#define PAGE_SIZE PFA_PAGE_SIZE
static void* alloc_pages(size_t num_pages)
{
    return pfa_alloc(num_pages);
}
#else
#include <syscalls.h>
#define PAGE_SIZE 0x1000
static void* alloc_pages(size_t num_pages)
{
    return sys_map_mem(NULL, num_pages * PAGE_SIZE);
}
#endif

void heap_init()
{
    base_address = alloc_pages(1);
    size = PAGE_SIZE;
    first_region = base_address;

    first_region->length = size;
    first_region->next_region = NULL;
    first_region->previous_region = NULL;
    first_region->free = true;
    first_region->chunk_id = 0;

    heap_mutex = 0;
}

void* heap_malloc(size_t size)
{
    if(size == 0)
        return NULL;

    mutex_acquire(&heap_mutex);

    retry: {}
    heap_region_t* current_region = first_region;

    while(true)
    {
        if(current_region->free && current_region->length >= size + sizeof(heap_region_t))
        {
            if(current_region->length >= size + sizeof(heap_region_t) * 2)
            {
                heap_region_t* new_region = (heap_region_t*)((uint64_t)current_region + size + sizeof(heap_region_t));
                new_region->length = current_region->length - (size + sizeof(heap_region_t));
                new_region->previous_region = current_region;
                new_region->next_region = current_region->next_region;
                new_region->free = true;
                new_region->chunk_id = current_region->chunk_id;

                if(new_region->next_region)
                    new_region->next_region->previous_region = new_region;

                current_region->next_region = new_region;
                current_region->length = size + sizeof(heap_region_t);
            }

            current_region->free = false;
            void* region = current_region + 1;
            mutex_release(&heap_mutex);
            return region;
        }
        else if(current_region->next_region == NULL)
            break;
        else
            current_region = current_region->next_region;
    }

    heap_region_t* new_region = alloc_pages(ALIGN_4K_UP(size + sizeof(heap_region_t)) / PAGE_SIZE);
    new_region->length = ALIGN_4K_UP(size + sizeof(heap_region_t));
    new_region->next_region = NULL;
    new_region->previous_region = current_region;
    new_region->free = true;
    new_region->chunk_id = current_region->chunk_id + 1;

    current_region->next_region = new_region;

    goto retry;
}

void heap_free(void* address)
{
    if(address == NULL)
        return;

    heap_region_t* region = (heap_region_t*)address - 1;

    mutex_acquire(&heap_mutex);

    region->free = true;

    if(region->previous_region != NULL && region->previous_region->free && region->chunk_id == region->previous_region->chunk_id)
    {
        region->previous_region->length += region->length;
        region->previous_region->next_region = region->next_region;
        if(region->next_region != NULL)
            region->next_region->previous_region = region->previous_region;
        region = region->previous_region;
    }

    if(region->next_region != NULL && region->next_region->free && region->chunk_id == region->next_region->chunk_id)
    {
        region->length += region->next_region->length;
        region->next_region = region->next_region->next_region;
        if(region->next_region != NULL)
            region->next_region->previous_region = region;
    }

    mutex_release(&heap_mutex);
}
