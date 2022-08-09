#include <memory/paging.h>
#include <memory/pfa.h>
#include <memory/mmap.h>
#include <linker.h>
#include <alignment.h>
#include <panic.h>

#define PML_PRESENT (1ull << 0)
#define PML_READWRITE (1ull << 1)
#define PML_PRIVILEGE (1ull << 2)
#define PML_WRITETHROUGH (1ull << 3)
#define PML_CACHEDISABLE (1ull << 4)
#define PML_ACCESSED (1ull << 5)
#define PML_DIRTY (1ull << 6)
#define PML_SIZE (1ull << 7)
#define PML_GLOBAL (1ull << 8)
#define PML_AVAILABLE (0b111ull << 9)
#define PML_ADDRESS (0xFFFFFFFFFFull << 12)
#define PML_EXECDISABLE (1ull << 63)

#define PML_GET_ADDRESS(entry) ((uint64_t*)((entry) & PML_ADDRESS))

#define PML_CLEAR_AVAILABLE(entry) ((entry) &= ~PML_AVAILABLE)
#define PML_SET_AVAILABLE(entry, val) ((entry) |= (((uint64_t)(val) << 9) & PML_AVAILABLE))
#define PML_UPDATE_AVAILABLE(entry, val) (PML_CLEAR_AVAILABLE(entry), PML_SET_AVAILABLE(entry, val))

#define PML_CLEAR_ADDRESS(entry) ((entry) &= ~PML_ADDRESS)
#define PML_SET_ADDRESS(entry, val) ((entry) |= ((uint64_t)(val) & PML_ADDRESS))
#define PML_UPDATE_ADDRESS(entry, val) (PML_CLEAR_ADDRESS(entry), PML_SET_ADDRESS(entry, val))

paging_data_t kernel_paging;

static void clean_structure(paging_data_t data)
{
    uint64_t* pml4 = (uint64_t*)data;

    for(uint16_t pml4_off = 0; pml4_off < 512; pml4_off++)
    {
        if((pml4[pml4_off] & PML_PRESENT) == 0)
            continue;

        uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
        bool empty_pdp = true;

        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
        {
            if((pdp[pdp_off] & PML_PRESENT) == 0)
                continue;
            empty_pdp = false;

            uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
            bool empty_pd = true;

            for(uint16_t pd_off = 0; pd_off < 512; pd_off++)
            {
                if((pd[pd_off] & PML_PRESENT) == 0)
                    continue;
                empty_pd = false;

                if((pd[pd_off] & PML_SIZE) != 0)
                    continue;

                uint64_t* pt = PML_GET_ADDRESS(pd[pd_off]);
                bool empty_pt = true;

                for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
                {
                    if((pt[pt_off] & PML_PRESENT) == 0)
                        continue;
                    empty_pt = false;
                    break;
                }

                if(empty_pt)
                {
                    pfa_free(PML_GET_ADDRESS(*pt), 1);
                    *pt = 0;
                }
            }

            if(empty_pd)
            {
                pfa_free(PML_GET_ADDRESS(*pd), 1);
                *pd = 0;
            }
        }

        if(empty_pdp)
        {
            pfa_free(PML_GET_ADDRESS(*pdp), 1);
            *pdp = 0;
        }
    }
}

void paging_init()
{
    kernel_paging = paging_create();
    paging_map(NULL, 0, 0, mmap_get()->total_memory, PAGE_PRIVILEGE_KERNEL);
    paging_enable(kernel_paging);
}

paging_data_t paging_create()
{
    uint64_t* pml4 = (uint64_t*)pfa_calloc(1);
    return (paging_data_t)pml4;
}

void paging_destroy(paging_data_t data)
{
    uint64_t* pml4 = (uint64_t*)data;
    for(uint16_t pml4_off = 0; pml4_off < 512; pml4_off++)
    {
        if((pml4[pml4_off] & PML_PRESENT )== 0)
            continue;

        uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
        for(uint16_t pdp_off = 0; pdp_off < 512; pdp_off++)
        {
            if((pdp[pdp_off] & PML_PRESENT) == 0)
                continue;

            uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
            for(uint16_t pd_pff = 0; pd_pff < 512; pd_pff++)
            {
                if((pd[pd_pff] & PML_PRESENT) == 0)
                    continue;

                pfa_free(PML_GET_ADDRESS(pd[pd_pff]), 1);
            }

            pfa_free(PML_GET_ADDRESS(pdp[pdp_off]), 1);
        }

        pfa_free(PML_GET_ADDRESS(pml4[pml4_off]), 1);
    }

    pfa_free((void*)pml4, 1);
}

void* paging_get_ph(paging_data_t data, const void* virtual_addr)
{
    data = data ?: kernel_paging;
    uint64_t vt = ALIGN_4K_DOWN(virtual_addr);
    uint64_t offset_4k = (uint64_t)virtual_addr - vt;
    uint64_t offset_2m = (uint64_t)virtual_addr - ALIGN_2M_DOWN(vt);

    uint64_t pml4_offset = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)vt >> 12) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
        return 0;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_offset]);
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
        return 0;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_offset]);
    if((pd[pd_offset] & PML_PRESENT) == 0)
        return 0;

    if(pd[pd_offset] & PML_SIZE)
        return (uint8_t*)PML_GET_ADDRESS(pd[pd_offset]) + offset_2m;

    uint64_t* pt = PML_GET_ADDRESS(pd[pd_offset]);
    if((pt[pt_offset] & PML_PRESENT) == 0)
        return 0;

    return (uint8_t*)PML_GET_ADDRESS(pt[pt_offset]) + offset_4k;
}

page_privilege_t paging_get_privilege(paging_data_t data, void* virtual_addr)
{
    data = data ?: kernel_paging;
    uint64_t vt = ALIGN_4K_DOWN(virtual_addr);

    uint64_t pml4_offset = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)vt >> 12) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
        return PAGE_PRIVILEGE_ERROR;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_offset]);
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
        return PAGE_PRIVILEGE_ERROR;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_offset]);
    if((pd[pd_offset] & PML_PRESENT) == 0)
        return PAGE_PRIVILEGE_ERROR;

    if(pd[pd_offset] & PML_SIZE)
        return pd[pd_offset] & PML_PRIVILEGE;

    uint64_t* pt = PML_GET_ADDRESS(pd[pd_offset]);
    if((pt[pt_offset] & PML_PRESENT) == 0)
        return PAGE_PRIVILEGE_ERROR;

    return pt[pt_offset] & PML_PRIVILEGE;
}

uint8_t paging_get_attr(paging_data_t data, void* vt_addr)
{
    data = data ?: kernel_paging;

    uint64_t vt = ALIGN_4K_DOWN(vt_addr);

    uint64_t pml4_off = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_off = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_off = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_off = ((uint64_t)vt >> 12) & 0x01FF;

    uint64_t* pml4 = (uint64_t*)data;
    if((pml4[pml4_off] & PML_PRESENT) == 0)
        return 0xFF;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
    if((pdp[pdp_off] & PML_PRESENT) == 0)
        return 0xFF;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
    if((pd[pd_off] & PML_PRESENT) == 0)
        return 0xFF;
    if(pt_off == -1)
        return (pd[pd_off] & PML_AVAILABLE) >> 9;

    uint64_t* pt = PML_GET_ADDRESS(pd[pd_off]);
    return (pt[pt_off] & PML_AVAILABLE) >> 9;
}

bool paging_set_attr(paging_data_t data, void* vt_addr, uint8_t attr)
{
    data = data ?: kernel_paging;

    uint64_t vt = ALIGN_4K_DOWN(vt_addr);

    uint64_t pml4_off = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_off = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_off = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_off = ((uint64_t)vt >> 12) & 0x01FF;

    uint64_t* pml4 = (uint64_t*)data;
    if((pml4[pml4_off] & PML_PRESENT) == 0)
        return false;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
    if((pdp[pdp_off] & PML_PRESENT) == 0)
        return false;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
    if((pd[pd_off] & PML_PRESENT) == 0)
        return false;
    if(pt_off != -1)
    {
        uint64_t* pt = PML_GET_ADDRESS(pd[pd_off]);
        PML_UPDATE_ADDRESS(pt[pt_off], attr);
    }
    else
        PML_UPDATE_AVAILABLE(pd[pd_off], attr);

    return true;
}

static void split_page(paging_data_t data, uint16_t pml4_offset, uint16_t pdp_offset, uint16_t pd_offset)
{
    uint64_t* huge_entry = &(((uint64_t***)data)[pml4_offset][pdp_offset][pd_offset]);
    void* ph_addr = PML_GET_ADDRESS(*huge_entry);
    uint64_t* pt = pfa_alloc(1);

    *huge_entry &= ~PML_SIZE;
    PML_UPDATE_ADDRESS(*huge_entry, pt);

    for(uint16_t pt_off = 0; pt_off < 512; pt_off++)
    {
        pt[pt_off] = *huge_entry;
        PML_UPDATE_ADDRESS(pt[pt_off], ph_addr + (pt_off * PFA_PAGE_SIZE));
    }
}

void paging_set_attr_range(paging_data_t data, void* vt_addr, size_t count, uint8_t attr)
{
    data = data ?: kernel_paging;

    uint64_t vt = ALIGN_4K_DOWN(vt_addr);

    uint64_t pml4_offset = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)vt >> 12) & 0x01FF;

    if(count > INT64_MAX)
        kernel_panic("paging_set_attr_range: count too big");
    int64_t amt_4k = count;

    uint64_t* pml4 = (uint64_t*)data;
    for(uint16_t pml4_off = pml4_offset; amt_4k > 0; pml4_off++)
    {
        if((pml4[pml4_off] & PML_PRESENT) == 0)
        {
            amt_4k -= 512 * 512 * 512;
            continue;
        }

        uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
        for(uint16_t pdp_off = pdp_offset; amt_4k > 0; pdp_off++)
        {
            if((pdp[pdp_off] & PML_PRESENT) == 0)
            {
                amt_4k -= 512 * 512;
                continue;
            }

            uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
            for(uint16_t pd_off = pd_offset; amt_4k > 0; pd_off++)
            {
                if((pd[pd_off] & PML_PRESENT) == 0)
                {
                    amt_4k -= 512;
                    continue;
                }

                if((pd[pd_off] & PML_SIZE) != 0)
                {
                    if(amt_4k >= 512)
                    {
                        PML_UPDATE_AVAILABLE(pd[pd_off], attr);
                        amt_4k -= 512;
                        continue;
                    }
                    else
                    {
                        split_page(data, pml4_off, pdp_off, pd_off);
                        pd = PML_GET_ADDRESS(pdp[pdp_off]);
                    }
                }

                uint64_t* pt = PML_GET_ADDRESS(pd[pd_off]);
                for(uint16_t pt_off = pt_offset; amt_4k > 0; pt_off++)
                {
                    if((pt[pt_off] & PML_PRESENT) == 0)
                    {
                        amt_4k--;
                        continue;
                    }

                    PML_UPDATE_AVAILABLE(pt[pt_off], attr);
                }
            }
        }
    }
}

bool paging_exists_page(paging_data_t data, uint16_t pml4_off, uint16_t pdp_off, uint16_t pd_off, uint16_t pt_off)
{
    data = data ?: kernel_paging;

    uint64_t* pml4 = (uint64_t*)data;
    if((pml4[pml4_off] & PML_PRESENT) == 0)
        return false;
    if(pdp_off == UINT16_MAX)
        return true;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_off]);
    if((pdp[pdp_off] & PML_PRESENT) == 0)
        return false;
    if(pd_off == UINT16_MAX)
        return true;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_off]);
    if((pd[pd_off] & PML_PRESENT) == 0)
        return false;
    if((pd[pd_off] & PML_SIZE) != 0)
        return true;
    if(pt_off == UINT16_MAX)
        return true;

    uint64_t* pt = PML_GET_ADDRESS(pd[pd_off]);
    return (pt[pt_off] & PML_PRESENT) != 0;
}

static bool attach_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool size)
{
    uint64_t vt = ALIGN_4K_DOWN(virtual_addr);
    uint64_t ph = ALIGN_4K_DOWN(physical_addr);

    uint64_t pml4_offset = (vt >> 39) & 0x01FF;
    uint64_t pdp_offset = (vt >> 30) & 0x01FF;
    uint64_t pd_offset = (vt >> 21) & 0x01FF;
    uint64_t pt_offset = (vt >> 12) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pdp = pfa_calloc(1);

        pml4[pml4_offset] = PML_PRESENT | PML_READWRITE | privilege;
        PML_UPDATE_ADDRESS(pml4[pml4_offset], pdp);
    }
    else if((pml4[pml4_offset] & PML_PRIVILEGE) != privilege)
        return false;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_offset]);
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pd = pfa_calloc(1);

        pdp[pdp_offset] = PML_PRESENT | PML_READWRITE | privilege;
        PML_UPDATE_ADDRESS(pdp[pdp_offset], pd);
    }
    else if((pdp[pdp_offset] & PML_PRIVILEGE) != privilege)
        return false;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_offset]);

    if(size)
    {
        if(pd[pd_offset] & PML_PRESENT)
            return false;

        pd[pd_offset] = PML_PRESENT | PML_READWRITE | PML_SIZE | privilege;
        PML_UPDATE_ADDRESS(pd[pd_offset], ph);

        return true;
    }

    if((pd[pd_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pt = pfa_calloc(1);

        pd[pd_offset] = PML_PRESENT | PML_READWRITE | privilege;
        PML_UPDATE_ADDRESS(pd[pd_offset], pt);
    }
    else if((pd[pd_offset] & PML_SIZE) || (pd[pd_offset] & PML_PRIVILEGE) != privilege)
        return false;

    uint64_t* pt = PML_GET_ADDRESS(pd[pd_offset]);
    if(pt[pt_offset] & PML_PRESENT)
        return false;

    pt[pt_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
    PML_UPDATE_ADDRESS(pt[pt_offset], ph);

    return true;
}

bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege)
{
    return attach_page(data ?: kernel_paging, physical_addr, virtual_addr, privilege, 0);
}

bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege)
{
    return attach_page(data ?: kernel_paging, physical_addr, virtual_addr, privilege, 1);
}

bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege)
{
    data = data ?: kernel_paging;

    uint64_t ph_aligned = ALIGN_4K_DOWN(physical_addr);
    uint64_t vh_aligned = ALIGN_4K_DOWN(virtual_addr);
    uint64_t length_aligned = ALIGN_4K_UP(length + physical_addr - ph_aligned);

    size_t amt_2mb = length_aligned / 0x200000;
    size_t amt_4kb = (length_aligned - (amt_2mb * 0x200000)) / 0x1000;

    bool error = false;

    while(amt_4kb > 0 && vh_aligned % 0x200000 != 0)
    {
        if(!attach_page(data, (void*)ph_aligned, (void*)vh_aligned, privilege, false))
            error = true;

        ph_aligned += 0x1000;
        vh_aligned += 0x1000;
        amt_4kb--;
    }

    while(amt_2mb > 0)
    {
        if(!attach_page(data, (void*)ph_aligned, (void*)vh_aligned, privilege, true))
            error = true;

        ph_aligned += 0x200000;
        vh_aligned += 0x200000;
        amt_2mb--;
    }

    while(amt_4kb > 0)
    {
        if(!attach_page(data, (void*)ph_aligned, (void*)vh_aligned, privilege, false))
            error = true;

        ph_aligned += 0x1000;
        vh_aligned += 0x1000;
        amt_4kb--;
    }

    return !error;
}

static bool detach_page(paging_data_t data, void* virtual_addr, bool size)
{
    uint64_t vt = ALIGN_4K_DOWN(virtual_addr);

    uint64_t pml4_offset = (vt >> 39) & 0x01FF;
    uint64_t pdp_offset = (vt >> 30) & 0x01FF;
    uint64_t pd_offset = (vt >> 21) & 0x01FF;
    uint64_t pt_offset = (vt >> 12) & 0x01FF;

    uint64_t* pml4 = (uint64_t*)data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
        return true;

    uint64_t* pdp = PML_GET_ADDRESS(pml4[pml4_offset]);
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
        return true;

    uint64_t* pd = PML_GET_ADDRESS(pdp[pdp_offset]);
    if((pd[pd_offset] & PML_PRESENT) == 0)
        return true;

    bool is_huge = (pd[pd_offset] & PML_SIZE) != 0;
    if(is_huge != size)
        return false;

    if(!size)
    {
        uint64_t* pt = PML_GET_ADDRESS(pd[pd_offset]);
        pt[pt_offset] = 0;
    }
    else
        pd[pd_offset] = 0;

    clean_structure(data);
    return true;
}

bool paging_detach_4kb_page(paging_data_t data, void* virtual_addr)
{
    return detach_page(data ?: kernel_paging, virtual_addr, false);
}

bool paging_detach_2mb_page(paging_data_t data, void* virtual_addr)
{
    return detach_page(data ?: kernel_paging, virtual_addr, true);
}

bool paging_unmap(paging_data_t data, void* virtual_addr, size_t length)
{
    data = data ?: kernel_paging;

    uint64_t vt = ALIGN_4K_DOWN(virtual_addr);
    uint64_t length_aligned = ALIGN_4K_UP(length + virtual_addr - vt);

    size_t amt_4k = length_aligned / 0x1000;

    bool error = false;
    while(amt_4k)
    {
        if(detach_page(data, (void*)vt, false))
        {
            vt += 0x1000;
            amt_4k--;
        }
        else if(amt_4k >= 512)
        {
            if(!detach_page(data, (void*)vt, true))
                error = true;
            amt_4k -= 512;
            vt += 0x200000;
        }
        else
        {
            uint64_t pml4_offset = ((uint64_t)vt >> 39) & 0x01FF;
            uint64_t pdp_offset = ((uint64_t)vt >> 30) & 0x01FF;
            uint64_t pd_offset = ((uint64_t)vt >> 21) & 0x01FF;

            split_page(data, pml4_offset, pdp_offset, pd_offset);
            if(!detach_page(data, (void*)vt, false))
                error = true;

            amt_4k--;
            vt += 0x1000;
        }
    }

    return !error;
}
