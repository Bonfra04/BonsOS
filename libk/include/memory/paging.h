#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum page_privilege
{
    PAGE_PRIVILEGE_KERNEL = 0 << 2,
    PAGE_PRIVILEGE_USER   = 1 << 2,

    PAGE_PRIVILEGE_ERROR  = 0xFF
} page_privilege_t;

typedef void* paging_data_t;

/**
 * @brief initializes paging and identity maps all memory
 */
void paging_init();

/**
 * @brief creates a paging structure
 * @return paging data to use for operations
 */
paging_data_t paging_create();

/**
 * @brief destroys a paging structure
 */
void paging_destroy(paging_data_t data);

/**
 * @brief translates a virtual address in physical address
 * @param[in] data Paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] vt The virtual address to translate
 * @return The translated physical address
 */
void* paging_get_ph(paging_data_t data, void* virtual_addr);

/**
 * @brief gets the set privilege of a virtual memory area
 * @param[in] data paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] virtual_addr virtual address to get the privilege of
 * @return the privilege of the virtual memory area
 */
page_privilege_t paging_get_privilege(paging_data_t data, void* virtual_addr);

/**
 * @brief gets the 3 bit attribute of the given page
 * @param[in] data paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] vt_addr virtual address of the page to get the attribute of
 * @return the attribute of the page (0xFFFF if invalid)
 */
uint8_t paging_get_attr(paging_data_t data, void* vt_addr);

/**
 * @brief sets the 3 bit attribute of the given page
 * @param[in] data paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] vt_addr virtual address of the page to set the attribute of
 * @param[in] attr the attribute to set
 * @return true if the attribute was set, false if the page was not mapped
 */
bool paging_set_attr(paging_data_t data, void* vt_addr, uint8_t attr);

/**
 * @brief sets the 3 bit attribute to `count` consecutive pages starting from the given page (if a page in this range is not mapped is ignored)
 * @param[in] data paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] vt_addr base virtual address of the page to set the attribute of
 * @param[in] count the amount of consecutive 4KB pages to set the attribute of
 * @param[in] attr the attribute to set
 */
void paging_set_attr_range(paging_data_t data, void* vt_addr, size_t count, uint8_t attr);

/**
 * @brief checks whether the given page index is mapped
 * @param[in] data paging hierarchy to fetch (NULL if refearing to kernel identity structure)
 * @param[in] pml4_off pml4 index
 * @param[in] pdp_off pdp index (UINT16_MAX for to ignore deppers levels)
 * @param[in] pd_off pd index (UINT16_MAX for to ignore deppers levels)
 * @param[in] pt_off pt index (UINT16_MAX for to ignore deppers levels)
 * @return true if the page is mapped, false otherwise
 */
bool paging_exists_page(paging_data_t data, uint16_t pml4_off, uint16_t pdp_off, uint16_t pd_off, uint16_t pt_off);

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] privilege page privilege
 * @return success
 */
bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege);

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] physical_addr 2MB aligned physical address to map form
 * @param[in] virtual_addr 2MB aligned virtual address to map to
 * @param[in] privilege page privilege
 * @return success
 */
bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege);

/**
 * @brief automatically creates and attaches pages to a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] length length in bytes of the range to map
 * @param[in] privilege page privilege
 */
bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege);

/**
 * @brief detaches a page from a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] virtual_addr 4KB aligned virtual address of the page to detach
 * @return success
 */
bool paging_detach_4kb_page(paging_data_t data, void* virtual_addr);

/**
 * @brief detaches a page from a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] virtual_addr 2MB aligned virtual address to map to
 * @return success
 */
bool paging_detach_2mb_page(paging_data_t data, void* virtual_addr);

/**
 * @brief automatically detaches pages from a paging structure
 * @param[in] data data structure to modify (NULL if refearing to kernel identity structure)
 * @param[in] virtual_addr 4KB aligned base virtual address to detach
 * @param[in] length length in bytes of the range to detach
 * @return success
 */
bool paging_unmap(paging_data_t data, void* virtual_addr, size_t length);

/**
 * @brief switch current paging hierarchy with the one provided as a parameter
 * @param[in] data data structure returned by paging_create()
 */
#define paging_enable(data) asm ("mov cr3, %0" : : "r" (data))

/**
 * combines the indices of a paging structure into a single virtual address
 * @param[in] pml4_off pml4 index
 * @param[in] pdp_off pdp index
 * @param[in] pd_off pd index
 * @param[in] pt_off pt index (-1 for huge page)
 * @return the virtual address
 */
#define paging_vt_from_indexes(pml4_off, pdp_off, pd_off, pt_off) ((void*)(((uint64_t)pml4_off << 39) | ((uint64_t)pdp_off << 30) | ((uint64_t)pd_off << 21) | ((uint64_t)pt_off << 12)))
