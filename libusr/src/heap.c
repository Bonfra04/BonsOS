#include <heap.h>
#include <string.h>
#include <syscalls.h>

typedef struct heap_region
{
    uint64_t length;
    struct heap_region* next_region;
    struct heap_region* previous_region;
    bool free;
} heap_region_t;

static uint64_t base_address;
static uint64_t size;
static heap_region_t* first_region;

static void combine_free_regions(heap_region_t* a, heap_region_t* b)
{
    if(a < b)
    {
        a->length += b->length;
        a->next_region = b->next_region;
    }
    else
    {
        b->length += a->length;
        b->next_region = b->next_region;
    }
}

void heap_init()
{
    size = 0x1000 * 256 * 20;
    base_address = map_mem(NULL, size);
    memset(base_address, 0, size);
    first_region = base_address;
    first_region->length = size;
    first_region->next_region = 0;
    first_region->previous_region = 0;
    first_region->free = true;
}

void* heap_malloc(size_t size)
{
    heap_region_t* currentRegion = first_region;

    while (currentRegion)
    {
        if(currentRegion->free && currentRegion->length >= size + sizeof(heap_region_t))
        {
            if(currentRegion->length > size + sizeof(heap_region_t))
            {
                heap_region_t* newRegion = (heap_region_t*)((uint64_t)currentRegion + size + sizeof(heap_region_t));
                newRegion->length = currentRegion->length - (size + sizeof(heap_region_t));
                newRegion->previous_region = currentRegion;
                newRegion->next_region = currentRegion->next_region;
                newRegion->free = true;

                currentRegion->next_region = newRegion;
                currentRegion->length = size + sizeof(heap_region_t);
            }

            currentRegion->free = false;
            return currentRegion + 1;
        }
        else
            currentRegion = currentRegion->next_region;
    }

    return 0;
}

void heap_free(void* address)
{
    if((uint64_t)address < base_address || (uint64_t)address > base_address + size)
        return; // not this heap memory

    heap_region_t* currentRegion = (heap_region_t*)address - 1;
    currentRegion->free = true;

    if(currentRegion->previous_region && currentRegion->previous_region->free)
        combine_free_regions(currentRegion, currentRegion->previous_region);

    if(currentRegion->next_region && currentRegion->next_region->free)
        combine_free_regions(currentRegion, currentRegion->next_region);
}