#pragma once

#include <stdint.h>
#include <stddef.h>

#define PFA_PAGE_SIZE 0x1000

/**
 * @brief initializes the page frame allocator
 */
void pfa_init();

/**
 * @brief allocates `count` consecutive pages
 * @param[in] count the number of pages to allocate
 * @return the address of the first page
 */
void* pfa_alloc(size_t count);

/**
 * @brief allocates zero initialized `count` consecutive pages
 * @param[in] count the number of pages to allocate
 * @return the address of the first page
 */
void* pfa_calloc(size_t count);

/**
 * @brief frees `count` consecutive pages
 * @param[in] ptr the address of the first page to free
 * @param[in] count the number of pages to free
 */
void pfa_free(void* ptr, size_t count);

/**
 * @brief initializes a memory region to be used by the page frame allocator (all regions are 4K aligned)
 * @param[in] base_address the base address of the region
 * @param[in] length the length of the region
 */
void pfa_init_region(void* base_address, size_t length);

/**
 * @brief deinitializes a memory region to rendering unreachable by the page frame allocator (all regions are 4K aligned)
 * @param[in] base_address the base address of the region
 * @param[in] length the length of the region
 */
void pfa_deinit_region(void* base_address, size_t length);
