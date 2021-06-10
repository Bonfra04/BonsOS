#include <memory/virtual_memory_manager.h>
#include <memory/page_frame_allocator.h>
#include <stdint.h>

#define PML_PRESENT (1ull << 0)
#define PML_READWRITE (1ull << 1)
#define PML_ADDRESS (0xFFFFFFFFFFull << 12)

static paging_data_t paging_data;

void vmm_set_paging(paging_data_t data)
{
    paging_data = data;
}

void vmm_destroy()
{
    // deallocates everything
    for(uint16_t pml4_off = 1; pml4_off < 512; pml4_off++)
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
            for(uint16_t pd_pff = 0; pdp_off < 512; pdp_off++)
                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                {
                    uint64_t address = paging_retrieve_node(paging_data, pml4_off, pdp_off, pdp_off, pt_off) & PML_ADDRESS;
                    if(address == 0)
                        continue;
                    paging_populate_node(paging_data, pml4_off, pdp_off, pdp_off, pt_off, 0);
                    pfa_free_page((void*)address);
                }
    // destroy actual paging
    paging_destroy(paging_data);
}

void* vmm_translate_vaddr(void* vaddr)
{
    return paging_get_ph(paging_data, vaddr);
}

void* vmm_alloc_page(page_privilege_t privilege)
{
    uint64_t* pml4 = (uint64_t*)paging_data;
    // skip 0th pml4 cause i feel like it
    for(uint16_t pml4_off = 1; pml4_off < 512; pml4_off++)
    {
        bool empty = pml4[pml4_off] == 0;
        if(!empty && !(pml4[pml4_off] & privilege))
            continue;

        uint64_t* pdp = empty ? 0 : (uint64_t*)(pml4[pml4_off] & PML_ADDRESS);
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
        {
            empty = empty ? true : pdp[pdp_off] == 0;
            if(!empty && !(pdp[pdp_off] & privilege))
                continue;
            
            uint64_t* pd = empty ? 0 : (uint64_t*)(pdp[pdp_off] & PML_ADDRESS);
            for(uint16_t pd_off = 0; pd_off < 512; pd_off++)
            {
                empty = empty ? true : pd[pd_off] == 0;
                if(!empty && !(pd[pd_off] & privilege))
                    continue;

                uint64_t* pt = empty ? 0 : (uint64_t*)(pd[pd_off] & PML_ADDRESS);
                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                {
                    empty = empty ? true : pt[pt_off] == 0;
                    if(!empty && !(pt[pt_off] & privilege))
                        continue;
                    
                    if(paging_retrieve_node(paging_data, pml4_off, pdp_off, pd_off, pt_off) == 0)
                    {
                        void* ph_addr = pfa_alloc_page();
                        uint64_t entry = (uint64_t)ph_addr | PML_PRESENT | PML_READWRITE | privilege;
                        paging_populate_node(paging_data, pml4_off, pdp_off, pd_off, pt_off, entry);
                        uint64_t vt_addr = (uint64_t)pml4_off << 39 | (uint64_t)pdp_off << 30 |
                            (uint64_t)pd_off << 21 | (uint64_t)pt_off << 12;
                        return (void*)vt_addr;
                    }
                }
            }
        }
    }
    return 0;
}

void vmm_free_page(void* page)
{
    uint64_t pml4_offset = ((uint64_t)page >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)page >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)page >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)page >> 12) & 0x01FF;

    void* ph_addr = paging_get_ph(paging_data, page);
    pfa_free_page(ph_addr);

    paging_populate_node(paging_data, pml4_offset, pdp_offset, pd_offset, pt_offset, 0);
}

void* vmm_alloc_pages(page_privilege_t privilege, size_t count)
{
    // skip 0th pml4 cause i feel like it
    for(uint16_t pml4_off = 1; pml4_off < 512; pml4_off++)
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
            for(uint16_t pd_off = 0; pd_off < 512; pd_off++)
                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                    if(paging_retrieve_node(paging_data, pml4_off, pdp_off, pd_off, pt_off) == 0)
                    {
                        void* ph_addr = pfa_alloc_pages(count);
                        uint64_t entry = (uint64_t)ph_addr | PML_PRESENT | PML_READWRITE | privilege;
                        paging_populate_node(paging_data, pml4_off, pdp_off, pd_off, pt_off, entry);
                        uint64_t vt_addr = (uint64_t)pml4_off << 39 | (uint64_t)pdp_off << 30 |
                            (uint64_t)pd_off << 21 | (uint64_t)pt_off << 12;
                        return (void*)vt_addr;
                    }
    return 0;
}

void vmm_free_pages(void* pages, size_t count)
{
    uint64_t pml4_offset = ((uint64_t)pages >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)pages >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)pages >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)pages >> 12) & 0x01FF;

    uint64_t ph_addr = paging_get_ph(paging_data, pages);
    pfa_free_pages((void*)ph_addr, count);

    paging_populate_node(paging_data, pml4_offset, pdp_offset, pd_offset, pt_offset, 0);
}