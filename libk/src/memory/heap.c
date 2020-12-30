#include <memory/heap.h>
#include <panic.h>

static heap_data_t* current_heap;

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

heap_data_t heap_create(void* base_address, uint64_t size)
{
    heap_data_t heap;

    heap.base_address = (uint64_t)base_address;
    heap.size = size;
    heap.first_region = base_address;
    heap.first_region->length = size;
    heap.first_region->next_region = 0;
    heap.first_region->previous_region = 0;
    heap.first_region->free = true;

    return heap;
}

void heap_activate(heap_data_t* heap)
{
    current_heap = heap;
}

void* heap_malloc(size_t size)
{
    if(!current_heap)
        kenrel_panic("No heap selected");

    heap_region_t* currentRegion = current_heap->first_region;

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
    if(!current_heap)
        kenrel_panic("No heap selected");

    if((uint64_t)address < current_heap->base_address || (uint64_t)address > current_heap->base_address + current_heap->size)
        return; // not this heap memory

    heap_region_t* currentRegion = (heap_region_t*)address - 1;
    currentRegion->free = true;

    if(currentRegion->previous_region && currentRegion->previous_region->free)
        combine_free_regions(currentRegion, currentRegion->previous_region);

    if(currentRegion->next_region && currentRegion->next_region->free)
        combine_free_regions(currentRegion, currentRegion->next_region);
}