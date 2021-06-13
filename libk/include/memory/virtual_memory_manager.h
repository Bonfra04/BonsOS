#pragma once

#include <stddef.h>
#include <memory/paging.h>

/**
 * @brief initialize the virtual memory manager with a paging hierarchy
 * @param[in] data paging hierarchy
 */
void vmm_set_paging(paging_data_t data);

/**
 * @brief destroys the selected paging hierarchy
 */
void vmm_destroy();

/**
 * @brief translates a virtual address to a physical address
 * @param[in] vaddr virtual address to translate
 * @return translated physical address 
 */
void* vmm_translate_vaddr(void* vaddr);

/**
 * @brief allocate a 4KB page in virtual space and returns its address
 * @param[in] privilege privilege to assign to the newly asllocate page
 */
void* vmm_alloc_page(page_privilege_t privilege);

/**
 * @brief frees a 4KB page in virtual space
 * @param[in] page virtual address returned by vmm_alloc_page
 */
void vmm_free_page(void* page);

/**
 * @brief allocate `count` contiguous 4KB pages in virtual space and returns the base address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @return base address of the pages
 */
void* vmm_alloc_pages(page_privilege_t privilege, size_t count);

/**
 * @brief maps `count` contiguous 4KB pages in virtual space to the provided physical address
 * @param[in] privilege privilege to assign to the newly allocated pages
 * @param[in] count amount of contiguous pages to allocate
 * @param[in] ph_addr physical address to map
 * @return base address of the pages
 */
void* vmm_assign_pages(page_privilege_t privilege, size_t count, void* ph_addr);

/**
 * @brief frees `count` contiguous 4KB pages in virtual space
 * @param[in] pages base virtual address returned by vmm_alloc_pages
 * @param[in] count amoutn of contiguous pages to free
 */
void vmm_free_pages(void* pages, size_t count);
