#include <memory/paging.h>
#include <stdint.h>
#include <string.h>
#include <memory/page_frame_allocator.h>

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

#define PML_CLEAR_AVAILABLE(entry) (entry &= ~PML_AVAILABLE)
#define PML_SET_AVAILABLE(entry, val) (entry |= (val & PML_AVAILABLE))
#define PML_UPDATE_AVAILABLE(entry, val) (PML_CLEAR_AVAILABLE(entry), PML_SET_AVAILABLE(entry, val))

#define PML_CLEAR_ADDRESS(entry) (entry &= ~PML_ADDRESS)
#define PML_SET_ADDRESS(entry, val) (entry |= (val & PML_ADDRESS))
#define PML_UPDATE_ADDRESS(entry, val) (PML_CLEAR_ADDRESS(entry), PML_SET_ADDRESS(entry, val))

static paging_data_t main_data;

paging_data_t paging_init(size_t memsize)
{
    main_data = 0;
    main_data = paging_create();

    extern char __kernel_start_addr[];
    extern char __kernel_end_addr[];
    void* kernel_start = __kernel_start_addr;
    void*  kernel_end = __kernel_end_addr;
    size_t kernel_end_aligned = (uint64_t)kernel_end + 0x200000 - (uint64_t)kernel_end % 0x200000;

    // globally map kernel code
    paging_map_global(kernel_start, kernel_start, (void*)kernel_end_aligned - kernel_start, PAGE_PRIVILEGE_KERNEL);

    // identity map entire memory
    paging_map(main_data, 0, 0, (size_t)kernel_start, PAGE_PRIVILEGE_KERNEL);
    paging_map(main_data, (void*)kernel_end_aligned, (void*)kernel_end_aligned, memsize - kernel_end_aligned, PAGE_PRIVILEGE_KERNEL);
    paging_enable(main_data);
    return main_data;
}

void paging_enable(paging_data_t data)
{
    asm volatile("mov cr3, %[addr]" : : [addr]"r"(data) : "memory");
}

void* paging_get_ph(paging_data_t data, void* vt)
{
    uint64_t pml4_offset = ((uint64_t)vt >> 39) & 0x01FF;
    uint64_t pdp_offset = ((uint64_t)vt >> 30) & 0x01FF;
    uint64_t pd_offset = ((uint64_t)vt >> 21) & 0x01FF;
    uint64_t pt_offset = ((uint64_t)vt >> 12) & 0x01FF;

    uint64_t* pml4 = data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
        return 0;

    uint64_t* pdp = (uint64_t*)((pml4[pml4_offset] & PML_ADDRESS));
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
        return 0;

    uint64_t* pd = (uint64_t*)((pdp[pdp_offset] & PML_ADDRESS));
    if((pd[pd_offset] & PML_PRESENT) == 0)
        return 0;

    if((pd[pd_offset] & PML_SIZE))
        return pd[pd_offset] & PML_ADDRESS;
    
    uint64_t* pt = (uint64_t*)((pd[pd_offset] & PML_ADDRESS));
    if((pt[pt_offset] & PML_PRESENT) == 0)
        return 0;

    return pt[pt_offset] & PML_ADDRESS;
}

static bool attach_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege, bool size, bool global)
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

    if(size)
    {
        if(pd[pd_offset] & PML_PRESENT)
        return false; // page altready exists

        pd[pd_offset] = (PML_PRESENT | PML_READWRITE | PML_SIZE) | privilege | (PML_GLOBAL * global);
        PML_UPDATE_ADDRESS(pd[pd_offset], (uint64_t)physical_addr);

        return true;
    }

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

    pt[pt_offset] = (PML_PRESENT | PML_READWRITE) | privilege;
    PML_UPDATE_ADDRESS(pt[pt_offset], (uint64_t)physical_addr);

    return true;
}

paging_data_t paging_create()
{
    uint64_t* pml4 = (uint64_t*)pfa_alloc_page();
    memset(pml4, 0, pfa_page_size());
    paging_data_t data = (paging_data_t)pml4;

    if(main_data)
    {
        uint64_t* main_pml4 = main_data;
        for(int i = 0; i < 512; i++)
            if(main_pml4[i] & PML_PRESENT)
            {
                uint64_t* main_pdp = (uint64_t*)(main_pml4[i] & PML_ADDRESS);
                for(int j = 0; j < 512; j++)
                    if(main_pdp[j] & PML_PRESENT)
                    {
                        uint64_t* main_pd = (uint64_t*)(main_pdp[j] & PML_ADDRESS);
                        for(int y = 0; y < 512; y++)
                            if(main_pd[y] & PML_PRESENT)
                            {
                                if(main_pd[y] & PML_SIZE)
                                {
                                    if(main_pd[y] & PML_GLOBAL)
                                    {
                                        uint64_t vt_addr = (uint64_t)i << 39 | (uint64_t)j << 30 | (uint64_t)y << 21;
                                        uint64_t ph_addr = main_pd[y] & PML_ADDRESS;
                                        page_privilege_t priv = main_pd[y] & PML_PRIVILEGE;
                                        attach_page(data, (void*)ph_addr, (void*)vt_addr, priv, 1, 1);
                                    }
                                }
                                else
                                {
                                    uint64_t* main_pt = (uint64_t*)(main_pd[y] & PML_ADDRESS);
                                    for(int l = 0; l < 512; l++)
                                        if(main_pt[l] & (PML_PRESENT | PML_GLOBAL))
                                        {
                                            uint64_t vt_addr = (uint64_t)(i) << 39 | (uint64_t)j << 30 | (uint64_t)y << 21 | (uint64_t)l << 12;
                                            uint64_t ph_addr = main_pt[l] & PML_ADDRESS;
                                            page_privilege_t priv = main_pt[l] & PML_PRIVILEGE;
                                            attach_page(data, (void*)ph_addr, (void*)vt_addr, priv, 0, 1);
                                        }  
                                }
                            }
                    }
            }
    }

    return data;
}

static bool map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege, bool global)
{
    // align length to 4KB
    if(length % 0x1000 != 0)
        length += 0x1000 - length % 0x1000;
    // align addr to 4KB
    if((uint64_t)physical_addr % 0x1000 != 0)
        physical_addr -= (uint64_t)physical_addr % 0x1000;
    if((uint64_t)virtual_addr % 0x1000 != 0)
        virtual_addr -= (uint64_t)virtual_addr % 0x1000;
    
    size_t amt_2mb = length / 0x200000;
    size_t amt_4kb = (length - (amt_2mb * 0x200000)) / 0x1000;

    uint64_t ph_addr = (uint64_t)physical_addr;
    uint64_t vt_addr = (uint64_t)virtual_addr;

    bool error = false;

    while(amt_4kb > 0 && vt_addr % 0x200000 != 0)
    {
        if(!attach_page(data, (void*)ph_addr, (void*)vt_addr, privilege, 0, global))
            error = true;
        ph_addr += 0x1000;
        vt_addr += 0x1000;
        amt_4kb--;
    }

    while(amt_2mb > 0)
    {
        if(!attach_page(data, (void*)ph_addr, (void*)vt_addr, privilege, 1, global))
            error = true;
        ph_addr += 0x200000;
        vt_addr += 0x200000;
        amt_2mb--;
    }

    while(amt_4kb > 0)
    {
        if(!attach_page(data, (void*)ph_addr, (void*)vt_addr, privilege, 0, global))
            error = true;
        ph_addr += 0x1000;
        vt_addr += 0x1000;
        amt_4kb--;
    }

    return !error;
}

bool paging_attach_4kb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege)
{
    return attach_page(data, physical_addr, virtual_addr, privilege, 0, false);
}

bool paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr, page_privilege_t privilege)
{
    return attach_page(data, physical_addr, virtual_addr, privilege, 1, false);
}

bool paging_map(paging_data_t data, void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege)
{
    return map(data, physical_addr, virtual_addr, length, privilege, false);
}

bool paging_map_global(void* physical_addr, void* virtual_addr, size_t length, page_privilege_t privilege)
{
    return map(main_data, physical_addr, virtual_addr, length, privilege, true);
}