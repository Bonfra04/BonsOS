#pragma once

#include <stddef.h>
#include <memory/paging.h>

/**
 * @brief destroys the selected paging hierarchy
 */
void vmm_destroy(paging_data_t paging_data);

/**
 * @brief translates a virtual address to a physical address
 * @param[in] paging paging hierarchy
 * @param[in] vaddr virtual address to translate
 * @return translated physical address 
 */
void* vmm_translate_vaddr(paging_data_t paging_data, void* vaddr);

/**
 * @brief allocate a 4KB page in virtual space and returns its address
 * @param[in] privilege privilege to assign to the newly asllocate page
 */
void* vmm_alloc_page(paging_data_t paging_data, page_privilege_t privilege);

/**
 * @brief frees a 4KB page in virtual space
 * @param[in] page virtual address returned by vmm_alloc_page
 */
void vmm_free_page(paging_data_t paging_data, void* page);

/**
 * @brief allocate `count` contiguous 4KB pages in virtual space and returns the base address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @return base address of the pages
 */
void* vmm_alloc_pages(paging_data_t paging_data, page_privilege_t privilege, size_t count);

/**
 * @brief maps `count` contiguous 4KB pages in virtual space to the provided physical address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @param[in] ph_addr physical address to map
 * @return virtual base address of the pages
 */
void* vmm_assign_pages(paging_data_t paging_data, page_privilege_t privilege, size_t count, void* ph_addr);

/**
 * @brief allocates `count` contigoud 4KB pages at the provided vt_addr
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contigous pages to allocate
 * @param[in] vt_addr virtual address to map
 * @return physical base address of the pages
 */
void* vmm_realloc_pages(paging_data_t paging_data, page_privilege_t privilege, size_t count, void* vt_addr);

/**
 * @brief frees `count` contiguous 4KB pages in virtual space
 * @param[in] pages base virtual address returned by vmm_alloc_pages
 * @param[in] count amoutn of contiguous pages to free
 */
void vmm_free_pages(paging_data_t paging_data, void* pages, size_t count);
