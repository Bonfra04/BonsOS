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
#define PML_SIZE (1ull << 7)
#define PML_AVAILABLE (0b111ull << 9)
#define PML_ADDRESS (0xFFFFFFFFFFull << 12)
#define PML_EXECDISABLE (1ull << 63)

#define PML_CLEAR_AVAILABLE(entry) (entry &= ~PML_AVAILABLE)
#define PML_SET_AVAILABLE(entry, val) (entry |= ((val << 9) & PML_AVAILABLE))
#define PML_UPDATE_AVAILABLE(entry, val) (PML_CLEAR_AVAILABLE(entry), PML_SET_AVAILABLE(entry, val));

#define PML_CLEAR_ADDRESS(entry) (entry &= ~PML_ADDRESS)
#define PML_SET_ADDRESS(entry, val) (entry |= ((val << 12) & PML_ADDRESS))
#define PML_UPDATE_ADDRESS(entry, val) (PML_CLEAR_ADDRESS(entry), PML_SET_ADDRESS(entry, val));

#define PT_UPDATE_ADDRESS(entry, val) PML_UPDATE_ADDRESS(entry, val)

/*
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
*/

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
    uint64_t* pml4 = (uint64_t*)pfa_alloc_page();
    memset(pml4, 0, pfa_page_size());

    /*
    extern void* kernel_start;
    extern void* kernel_end;
    size_t kernel_size = (ptrdiff_t)(kernel_end - kernel_end);
    size_t page_count = kernel_size / pfa_page_size();
    page_count += (uint64_t)kernel_size % pfa_page_size() != 0;
    for(size_t i = 0; i < page_count; i++)
        paging_attach_page((paging_data_t)pml4, kernel_start + pfa_page_size() * i, kernel_start + pfa_page_size() * i);
    */

    return (paging_data_t)pml4;
}

/*
void paging_attach_page(paging_data_t data, void* physical_addr, void* virtual_addr)
{
    uint64_t pml4_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 39;
    uint64_t pdp_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 30;
    uint64_t pd_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 21;
    uint64_t pt_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 12;

    pml4_entry_t* pml4 = (pml4_entry_t*)data;
    if(pml4[pml4_offset].present == 0)
    {
        pdp_entry_t* pdp = pfa_alloc_page();
        memset(pdp, 0, pfa_page_size());

        pml4[pml4_offset].present = 1;
        pml4[pml4_offset].readwrite = 1;
        pml4[pml4_offset].user = 0; // 1 if using usermode
        pml4[pml4_offset].execdisable = 0;
        pml4[pml4_offset].address = (uint64_t)pdp >> 12;
    }

    pdp_entry_t* pdp = (pdp_entry_t*)(uint64_t)pml4[pml4_offset].address;
    if(pdp[pdp_offset].present == 0)
    {
        pdp_entry_t* pd = pfa_alloc_page();
        memset(pd, 0, pfa_page_size());

        pdp[pdp_offset].present = 1;
        pdp[pdp_offset].readwrite = 1;
        pdp[pdp_offset].user = 0; // 1 if using usermode
        pdp[pdp_offset].execdisable = 0;
        pdp[pdp_offset].address = (uint64_t)pd >> 12;
    }

    pd_entry_t* pd = (pd_entry_t*)(uint64_t)pdp[pdp_offset].address;
    if(pd[pd_offset].present == 0)
    {
        pdp_entry_t* pt = pfa_alloc_page();
        memset(pt, 0, pfa_page_size());

        pd[pd_offset].present = 1;
        pd[pd_offset].readwrite = 1;
        pd[pd_offset].user = 0; // 1 if using usermode
        pd[pd_offset].execdisable = 0;
        pd[pd_offset].address = (uint64_t)pt >> 12;
    }

    pt_entry_t* pt = (pt_entry_t*)(uint64_t)pd[pd_offset].address;
    pt[pt_offset].present = 1;
    pt[pt_offset].readwrite = 1;
    pt[pt_offset].user = 0; // 1 if using usermode
    pt[pt_offset].address = (uint64_t)physical_addr >> 12;
}
*/

void paging_attach_2mb_page(paging_data_t data, void* physical_addr, void* virtual_addr)
{
    uint64_t pml4_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 39;
    uint64_t pdp_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 30;
    uint64_t pd_offset = (uint64_t)virtual_addr & UINT64_C(0x01FF) >> 21;

    uint64_t* pml4 = (uint64_t*)data;
    if((pml4[pml4_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pdp = pfa_alloc_page();
        memset(pdp, 0, pfa_page_size());

        pml4[pml4_offset] |= PML_PRESENT;
        pml4[pml4_offset] |= PML_READWRITE;
        pml4[pml4_offset] |= ~PML_USER; // set if using usermode
        PML_UPDATE_ADDRESS(pml4[pml4_offset], (uint64_t)pdp);
    }

    uint64_t* pdp = (uint64_t*)((pml4[pml4_offset] & PML_ADDRESS) >> 12);
    if((pdp[pdp_offset] & PML_PRESENT) == 0)
    {
        uint64_t* pd = pfa_alloc_page();
        memset(pd, 0, pfa_page_size());

        pdp[pdp_offset] |= PML_PRESENT;
        pdp[pdp_offset] |= PML_READWRITE;
        pdp[pdp_offset] |= ~PML_USER; // set if using usermode
        PML_UPDATE_ADDRESS(pdp[pdp_offset], (uint64_t)pd);
    }

    uint64_t* pd = (uint64_t*)((pdp[pdp_offset] & PML_ADDRESS) >> 12);
    pd[pd_offset] |= PML_PRESENT;
    pd[pd_offset] |= PML_READWRITE;
    pd[pd_offset] |= ~PML_USER; // set if using usermode
    pd[pd_offset] |= PML_SIZE; // 2mb page
    PML_UPDATE_ADDRESS(pd[pd_offset], (uint64_t)physical_addr);
}