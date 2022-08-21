#include <memory/vmm.h>
#include <memory/pfa.h>
#include <panic.h>

#include <atomic/mutex.h>

#define VMM_ALLOC (0b111)

static mutex_t assign_mutex = 0;
static mutex_t alloc_mutex = 0;

void vmm_destroy(paging_data_t paging_data)
{
    for(uint16_t pml4_off = 1; pml4_off < 512; pml4_off++)
    {
        if(!paging_exists_page(paging_data, pml4_off, UINT16_MAX, UINT16_MAX, UINT16_MAX))
            continue;
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
        {
            if(!paging_exists_page(paging_data, pml4_off, pdp_off, UINT16_MAX, UINT16_MAX))
                continue;

            for(uint16_t pd_off = 0; pdp_off < 512; pdp_off++)
            {
                if(!paging_exists_page(paging_data, pml4_off, pdp_off, pdp_off, UINT16_MAX))
                    continue;

                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                {
                    if(!paging_exists_page(paging_data, pml4_off, pdp_off, pd_off, pt_off))
                        continue;

                    void* vt = paging_vt_from_indexes(pml4_off, pdp_off, pd_off, pt_off);
                    void* ph = paging_get_ph(paging_data, vt);
                    uint16_t attr = paging_get_attr(paging_data, vt);

                    if(attr == VMM_ALLOC)
                        pfa_free(ph, 1);
                }
            }
        }
    }

    paging_destroy(paging_data);
}

void* vmm_assign_pages(paging_data_t paging_data, page_privilege_t privilege, size_t count, void* ph_addr)
{
    size_t found = 0;
    uint16_t candidate_pml4;
    uint16_t candidate_pdp;
    uint16_t candidate_pd;
    uint16_t candidate_pt;

    mutex_acquire(&assign_mutex);

    // TODO: this will succeed if you ask for a privilege different than the pml4's one.
    for(uint16_t pml4_off = 1; pml4_off < 512; pml4_off++)
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
            for(uint16_t pd_off = 0; pd_off < 512; pd_off++)
                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                    if(!paging_exists_page(paging_data, pml4_off, pdp_off, pd_off, pt_off))
                    {
                        if(found == 0)
                        {
                            candidate_pml4 = pml4_off;
                            candidate_pdp = pdp_off;
                            candidate_pd = pd_off;
                            candidate_pt = pt_off;
                        }
                        if(++found == count)
                            goto alloc;
                    }
                    else
                        found = 0;

    mutex_release(&assign_mutex);
    return NULL;

    alloc: {}
    uint64_t vt_addr = (uint64_t)paging_vt_from_indexes(candidate_pml4, candidate_pdp, candidate_pd, candidate_pt);

    if(!paging_map(paging_data, ph_addr, (void*)vt_addr, count * PFA_PAGE_SIZE, privilege))
        return vt_addr = NULL;

    mutex_release(&assign_mutex);
    return (void*)vt_addr;
}

void* vmm_alloc(paging_data_t paging_data, page_privilege_t privilege, size_t count)
{
    void* ph_addr = pfa_alloc(count);

    mutex_acquire(&alloc_mutex);
    void* vt_addr = vmm_assign_pages(paging_data, privilege, count, ph_addr);

    if(vt_addr == NULL)
        pfa_free(ph_addr, count);
    else
        paging_set_attr_range(paging_data, vt_addr, count, VMM_ALLOC);

    mutex_release(&alloc_mutex);
    return vt_addr;
}

void vmm_free(paging_data_t paging_data, void* pages, size_t count)
{
    for(size_t i = 0; i < count; i++)
    {
        uint64_t vt_addr = (uint64_t)pages + i * PFA_PAGE_SIZE;
        
        mutex_acquire(&alloc_mutex);
        void* ph_addr = paging_get_ph(paging_data, (void*)vt_addr);
        uint8_t attr = paging_get_attr(paging_data, (void*)vt_addr);

        if(attr == VMM_ALLOC)
        {
            paging_unmap(paging_data, (void*)vt_addr, PFA_PAGE_SIZE);
            pfa_free(ph_addr, 1);
        }
        mutex_release(&alloc_mutex);
    }
}
