#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum page_privilege
{
    PAGE_PRIVILEGE_KERNEL = 0 << 2,
    PAGE_PRIVILEGE_USER   = 1 << 2,
} page_privilege_t;

typedef void* paging_data_t;

/**
 * @brief creates a paging structure
 * @return paging data to use for operations
 */
paging_data_t paging_create();

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] flags flags to apply to the page
 * @return success
 */
bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool global);

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 2MB aligned physical address to map form
 * @param[in] virtual_addr 2MB aligned virtual address to map to
 * @param[in] flags flags to apply to the page
 * @return success
 */
bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool global);

/**
 * @brief automatically creates and attaches pages to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] flags flags to apply to the page
 */
bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege, bool global);

/**
 * @brief switch current paging hierarchy with the one provided as a parameter
 * @param[in] data data structure returned by paging_create()
 */
void paging_enable(paging_data_t data);