#include "paging.h"

#include "../lib/stdint.h"
#include "../lib/string.h"

static uint64_t __attribute__ ((aligned (0x1000))) pml4[512];
static uint64_t __attribute__ ((aligned (0x1000))) pdp[512];

#define PML_PRESENT (1ull << 0)
#define PML_READWRITE (1ull << 1)
#define PML_SIZE (1ull << 7)

void paging_identity_map()
{
    memset(pml4, 0, sizeof(pml4));
    memset(pdp, 0, sizeof(pdp));

    pml4[0] = PML_PRESENT | PML_READWRITE | (uint64_t)pdp;

    for(size_t i = 0; i < 512; i++)
        pdp[i] = PML_PRESENT | PML_READWRITE | PML_SIZE | (i * 0x1000);

    asm volatile("mov cr3, %[addr]" : : [addr]"r"(pml4) : "memory");
}