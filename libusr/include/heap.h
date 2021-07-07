#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Initialize the heap at a specific memory address.
 */
void heap_init();

/**
 * @brief Allocates a specific amount of bytes on the heap. 
 * @param[in] size Size of memory to allocate.
 */
void* heap_malloc(size_t size);

/**
 * @brief Frees a specific a memory block from the heap. 
 * @param[in] address Address of the block of memory to deallocate (retrived from heap_malloc).
 */
void heap_free(void* address);
