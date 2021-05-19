#include <memory/paging.h>
#include <stdint.h>
#include <string.h>
#include <memory/page_frame_allocator.h>

typedef struct pml4_entry
{
   uint8_t present : 1;
   uint8_t readwrite : 1;
   uint8_t user : 1;
   uint8_t writethrough : 1;
   uint8_t cachedisable : 1;
   uint8_t accessed : 1;
   uint8_t reserved0 : 1;
   uint8_t size : 1;
   uint8_t reserved1 : 1;
   uint8_t available : 3;
   uint64_t address : 40;
   uint16_t reserved2 : 11;
   uint8_t execdisable : 1;
} __attribute__((packed)) pml4_entry_t;

typedef struct pdp_entry
{
   uint8_t present : 1;
   uint8_t readwrite : 1;
   uint8_t user : 1;
   uint8_t writethrough : 1;
   uint8_t cachedisable : 1;
   uint8_t accessed : 1;
   uint8_t reserved0 : 1;
   uint8_t size : 1;
   uint8_t reserved1 : 1;
   uint8_t available : 3;
   uint64_t address : 40;
   uint16_t reserved2 : 11;
   uint8_t execdisable : 1;
} __attribute__((packed)) pdp_entry_t;

typedef struct pd_entry
{
   uint8_t present : 1;
   uint8_t readwrite : 1;
   uint8_t user : 1;
   uint8_t writethrough : 1;
   uint8_t cachedisable : 1;
   uint8_t accessed : 1;
   uint8_t reserved0 : 1;
   uint8_t size : 1;
   uint8_t reserved1 : 1;
   uint8_t available : 3;
   uint64_t address : 40;
   uint16_t reserved2 : 11;
   uint8_t execdisable : 1;
} __attribute__((packed)) pd_entry_t;

typedef struct pt_entry
{
   uint8_t present : 1;
   uint8_t readwrite : 1;
   uint8_t user : 1;
   uint8_t writethrough : 1;
   uint8_t cachedisable : 1;
   uint8_t accessed : 1;
   uint8_t dirty : 1;
   uint8_t reserved0 : 1;
   uint64_t global : 1;
   uint8_t available : 3;
   uint64_t address : 40;
   uint16_t reserved1 : 11;
   uint8_t execdisable : 1;
} __attribute__((packed)) pt_entry_t;

paging_data_t paging_create()
{
    pml4_entry_t* pml4 = (pml4_entry_t*)pfa_alloc_page();
    memset(pml4, 0, sizeof(pml4_entry_t) * 512);

    extern void* kernel_start;
    extern void* kernel_end;
    size_t kernel_size = (ptrdiff_t)(kernel_end - kernel_end);
    size_t page_count = kernel_size / pfa_page_size();
    page_count += (uint64_t)kernel_size % pfa_page_size() != 0;
    for(size_t i = 0; i < page_count; i++)
        paging_attach_page((paging_data_t)pml4, kernel_start + pfa_page_size() * i, kernel_start + pfa_page_size() * i);

    return (paging_data_t)pml4;
}

void paging_attach_page(paging_data_t data, void* physical_addr, void* virtual_addr)
{
    uint64_t pml4_offset = (uint64_t)virtual_addr & (0x01FF) >> 39;
    uint64_t pdp_offset = (uint64_t)virtual_addr & (0x01FF) >> 30;
    uint64_t pd_offset = (uint64_t)virtual_addr & (0x01FF) >> 21;
    uint64_t pt_offset = (uint64_t)virtual_addr & (0x01FF) >> 12;
    uint64_t page_offset = (uint64_t)virtual_addr & (0xFFF) >> 0;

    pml4_entry_t* pml4 = (pml4_entry_t*)data;
    if(pml4[pml4_offset].present == 0)
    {
        pdp_entry_t* pdp = pfa_alloc_page();
        memset(pdp, 0, pfa_page_size);

        pml4[pml4_offset].present = 1;
        pml4[pml4_offset].readwrite = 1;
        pml4[pml4_offset].user = 0; // 1 if using usermode
        pml4[pml4_offset].execdisable = 0;
        pml4[pml4_offset].address = (uint64_t)pdp >> 12;
    }

    pdp_entry_t* pdp = (pdp_entry_t*)pml4[pml4_offset].address;
    if(pdp[pdp_offset].present == 0)
    {
        pdp_entry_t* pd = pfa_alloc_page();
        memset(pd, 0, pfa_page_size);

        pdp[pdp_offset].present = 1;
        pdp[pdp_offset].readwrite = 1;
        pdp[pdp_offset].user = 0; // 1 if using usermode
        pdp[pdp_offset].execdisable = 0;
        pdp[pdp_offset].address = (uint64_t)pd >> 12;
    }

    pd_entry_t* pd = (pd_entry_t*)pdp[pdp_offset].address;
    if(pd[pd_offset].present == 0)
    {
        pdp_entry_t* pt = pfa_alloc_page();
        memset(pt, 0, pfa_page_size);

        pd[pd_offset].present = 1;
        pd[pd_offset].readwrite = 1;
        pd[pd_offset].user = 0; // 1 if using usermode
        pd[pd_offset].execdisable = 0;
        pd[pd_offset].address = (uint64_t)pt >> 12;
    }

    pt_entry_t* pt = (pt_entry_t*)pd[pd_offset].address;
    if(pt[pt_offset].present == 0)
    {
        pt[pt_offset].present = 1;
        pt[pt_offset].readwrite = 1;
        pt[pt_offset].user = 0; // 1 if using usermode
        pt[pt_offset].address = physical_addr;
    }
}