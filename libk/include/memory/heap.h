#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct heap_region
{
    uint64_t length;
    struct heap_region* next_region;
    struct heap_region* previous_region;
    bool free;
} heap_region_t;

typedef struct
{
    uint64_t base_address;
    uint64_t size;
    heap_region_t* first_region;
} heap_data_t;

/**
 * @brief Creates a sized heap in a specific memory address.
 * @param[in] base_address Base address of the heap.
 * @param[in] size Size in bytes the heap.
 * @return The heap object. (Heap has to be activated)
 */
heap_data_t heap_create(void* base_address, uint64_t size);

/**
 * @brief Activates the provided heap.
 * @param[in] base_address Heap object to activate.
 */
void heap_activate(heap_data_t* heap);

/**
 * @brief Allocates a specific amount of bytes on the activated heap. 
 * @param[in] size Size of memory to allocate.
 */
void* heap_malloc(size_t size);

/**
 * @brief Frees a specific a memory block from the activated heap. 
 * @param[in] address Address of the block of memory to deallocate (retrived from heap_malloc).
 */
void heap_free(void* address);