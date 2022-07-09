#pragma once

#include <memory/paging.h>

#include <stddef.h>

/**
 * @brief destroys the selected paging hierarchy
 */
void vmm_destroy(paging_data_t paging_data);

/**
 * @brief maps `count` contiguous 4KB pages in virtual space to the provided physical address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @param[in] ph_addr physical address to map
 * @return virtual base address of the pages
 */
void* vmm_assign_pages(paging_data_t paging_data, page_privilege_t privilege, size_t count, void* ph_addr);

/**
 * @brief allocates `count` contiguous 4KB pages in virtual space and returns the base address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @return virtual base address of the pages
 */
void* vmm_alloc(paging_data_t paging_data, page_privilege_t privilege, size_t count);

/**
 * @brief frees `count` contiguous 4KB pages in virtual space
 * @param[in] vt_addr base virtual address of the pages
 * @param[in] count amount of contiguous pages to free
 */
void vmm_free(paging_data_t paging_data, void* vt_addr, size_t count);
