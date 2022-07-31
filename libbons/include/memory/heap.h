#pragma once

#include <stddef.h>

/**
 * @brief initializes the heap
 */
void heap_init();

/**
 * @brief allocates a specific amount of bytes on the heap
 * @param[in] size size in bytes of memory to allocate
 */
void* heap_malloc(size_t size);

/**
 * @brief frees a specific a memory block from the heap
 * @param[in] address address of the block of memory to deallocate
 */
void heap_free(void* address);
