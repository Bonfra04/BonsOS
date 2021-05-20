#pragma once

typedef void* paging_data_t;

/**
 * @brief creates a paging structure
 * @return paging data to use for operations
 */
paging_data_t paging_create();

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4K aligned physical address to map form
 * @param[in] virtual_addr 4K aligned virtual address to map to
 */
void paging_attach_page(paging_data_t data, void* physical_addr, void* virtual_addr);

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 2MB aligned physical address to map form
 * @param[in] virtual_addr 2MB aligned virtual address to map to
 */
void paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr);