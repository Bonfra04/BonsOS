#include <memory/paging.h>
#include <stdint.h>
#include <string.h>
#include <memory/page_frame_allocator.h>

#define PML_PRESENT (1ull << 0)
#define PML_READWRITE (1ull << 1)
#define PML_USER (1ull << 2)
#define PML_WRITETHROUGH (1ull << 3)
#define PML_CACHEDISABLE (1ull << 4)
#define PML_ACCESSED (1ull << 5)
#define PML_DIRTY (1ull << 6)
#define PML_SIZE (1ull << 7)
#define PML_GLOBAL (1ull << 8)
#define PML_AVAILABLE (0b111ull << 9)
#define PML_ADDRESS (0xFFFFFFFFFFull << 12)
#define PML_EXECDISABLE (1ull << 63)

#define PML_CLEAR_AVAILABLE(entry) (entry &= ~PML_AVAILABLE)
#define PML_SET_AVAILABLE(entry, val) (entry |= (val & PML_AVAILABLE))
#define PML_UPDATE_AVAILABLE(entry, val) (PML_CLEAR_AVAILABLE(entry), PML_SET_AVAILABLE(entry, val))

#define PML_CLEAR_ADDRESS(entry) (entry &= ~PML_ADDRESS)
#define PML_SET_ADDRESS(entry, val) (entry |= (val & PML_ADDRESS))
#define PML_UPDATE_ADDRESS(entry, val) (PML_CLEAR_ADDRESS(entry), PML_SET_ADDRESS(entry, val))

void paging_enable(paging_data_t data)
{
    asm volatile("mov cr3, %[addr]" : : [addr]"r"(data) : "memory");
}

paging_data_t paging_create()
{
    uint64_t* pml4 = (uint64_t*)pfa_alloc_page();
    memset(pml4, 0, pfa_page_size());
    return (paging_data_t)pml4;
}

bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool global)
{
    uint64_t pml4_offset = ((uint64_t)virtual_addr >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)virtual_addr >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)virtual_addr >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)virtual_addr >> 12) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pdp = pfa_alloc_page();
        memset(pdp, 0, pfa_page_size());

        pml4[pml4_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
        PML_UPDATE_ADDRESS(pml4[pml4_offset], (uint64_t)pdp);
    }

    uint64_t* pdp = (uint64_t*)((pml4[pml4_offset] & PML_ADDRESS));
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pd = pfa_alloc_page();
        memset(pd, 0, pfa_page_size());

        pdp[pdp_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
        PML_UPDATE_ADDRESS(pdp[pdp_offset], (uint64_t)pd);
    }

    uint64_t* pd = (uint64_t*)((pdp[pdp_offset] & PML_ADDRESS));
    if((pd[pd_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pt = pfa_alloc_page();
        memset(pt, 0, pfa_page_size());

        pd[pd_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
        PML_UPDATE_ADDRESS(pd[pd_offset], (uint64_t)pt);
    }
    else if(pd[pd_offset] & PML_SIZE)
        return false; // pd is huge page;

    uint64_t* pt = (uint64_t*)((pd[pd_offset] & PML_ADDRESS));
    if(pt[pt_offset] & PML_PRESENT)
        return false; // page altready exists

    pt[pt_offset] = (PML_PRESENT | PML_READWRITE) | privilege | (PML_GLOBAL * global);
    PML_UPDATE_ADDRESS(pt[pt_offset], (uint64_t)physical_addr);

    return true;
}

bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool global)
{
    uint64_t pml4_offset = ((uint64_t)virtual_addr >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)virtual_addr >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)virtual_addr >> 21) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pdp = pfa_alloc_page();
        memset(pdp, 0, pfa_page_size());

        pml4[pml4_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
        PML_UPDATE_ADDRESS(pml4[pml4_offset], (uint64_t)pdp);
    }

    uint64_t* pdp = (uint64_t*)((pml4[pml4_offset] & PML_ADDRESS));
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pd = pfa_alloc_page();
        memset(pd, 0, pfa_page_size());

        pdp[pdp_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
        PML_UPDATE_ADDRESS(pdp[pdp_offset], (uint64_t)pd);
    }

    uint64_t* pd = (uint64_t*)((pdp[pdp_offset] & PML_ADDRESS));
    if(pd[pd_offset] & PML_PRESENT)
        return false; // page altready exists

    pd[pd_offset] = (PML_PRESENT | PML_READWRITE | PML_SIZE) | privilege | (PML_GLOBAL * global);
    PML_UPDATE_ADDRESS(pd[pd_offset], (uint64_t)physical_addr);

    return true;
}

bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege, bool global)
{
    // align length to 4KB
    if(length % 0x1000 != 0)
        length += 0x1000 - length % 0x1000;
    // align addr to 4KB
    if((uint64_t)physical_addr % 0x1000 != 0)
        physical_addr += (uint64_t)physical_addr % 0x1000;
    if((uint64_t)virtual_addr % 0x1000 != 0)
        virtual_addr += (uint64_t)virtual_addr % 0x1000;
    
    size_t amt_2mb = length / 0x200000;
    size_t amt_4kb = (length - (amt_2mb * 0x200000)) / 0x1000;

    uint64_t ph_addr = physical_addr;
    uint64_t vt_addr = virtual_addr;

    bool error = false;

    while(amt_4kb > 0 && vt_addr % 0x200000 != 0)
    {
        if(!paging_attach_4kb_page(data, ph_addr, vt_addr, privilege, global))
            error = true;
        ph_addr += 0x1000;
        vt_addr += 0x1000;
        amt_4kb--;
    }

    while(amt_2mb > 0)
    {
        if(!paging_attach_2mb_page(data, ph_addr, vt_addr, privilege, global))
            error = true;
        ph_addr += 0x200000;
        vt_addr += 0x200000;
        amt_2mb--;
    }

    while(amt_4kb > 0)
    {
        if(!paging_attach_4kb_page(data, ph_addr, vt_addr, privilege, global))
            error = true;
        ph_addr += 0x1000;
        vt_addr += 0x1000;
        amt_4kb--;
    }

    return !error;
}