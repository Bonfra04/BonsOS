#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum page_privilege
{
    PAGE_PRIVILEGE_KERNEL = 0 << 2,
    PAGE_PRIVILEGE_USER   = 1 << 2,
} page_privilege_t;

typedef void* paging_data_t;

/**
 * @brief initialize paging and enables a full identity map
 * @param[in] memsize memory size in bytes
 * @return identity paging data
 */
paging_data_t paging_init(size_t memsize);

/**
 * @brief creates a paging structure
 * @return paging data to use for operations
 */
paging_data_t paging_create();

/**
 * @brief translate a virtual address in physical address
 * @param[in] data paging hierachy to interrogate
 * @param[in] vt the virtual address to transalte
 * @return the translated physical address
 */
void* paging_get_ph(paging_data_t data, void* vt);

/**
 * @brief populates a specific node of the hierarchy
 * @param[in] data paging hierachy to interrogate
 * @param[in] pml4_off pml4 index
 * @param[in] pdp_off pdp index
 * @param[in] pd_off pd index
 * @param[in] pt_off pt index
 */
void paging_populate_node(paging_data_t data, uint16_t pml4_off, uint16_t pdp_off, uint16_t pd_off, uint16_t pt_off, uint64_t value);

/**
 * @brief retrieve a specific node of the hierarchy
 * @param[in] data paging hierachy to interrogate
 * @param[in] pml4_off pml4 index
 * @param[in] pdp_off pdp index
 * @param[in] pd_off pd index
 * @param[in] pt_off pt index
 * @return the value of the node
 */
uint64_t paging_retrieve_node(paging_data_t data, uint16_t pml4_off, uint16_t pdp_off, uint16_t pd_off, uint16_t pt_off);


/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] privilege page privilege
 * @return success
 */
bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege);

/**
 * @brief attaches a page to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 2MB aligned physical address to map form
 * @param[in] virtual_addr 2MB aligned virtual address to map to
 * @param[in] privilege page privilege
 * @return success
 */
bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege);

/**
 * @brief automatically creates and attaches pages to a paging structure
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] privilege page privilege
 */
bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege);

/**
 * @brief maps globally a memory area (global maps will exists only on hierarchies created after this call)
 * @param[in] data data structure returned by paging_create()
 * @param[in] physical_addr 4KB aligned physical address to map form
 * @param[in] virtual_addr 4KB aligned virtual address to map to
 * @param[in] privilege page privilege
 */
bool paging_map_global(void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege);

/**
 * @brief switch current paging hierarchy with the one provided as a parameter
 * @param[in] data data structure returned by paging_create()
 */
void paging_enable(paging_data_t data);