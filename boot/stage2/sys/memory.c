#include "memory.h"
#include "realmode.h"
#include "../lib/string.h"

#define MAX_ENTRIES 256

mmap_entry_t memory_map[MAX_ENTRIES];
static size_t num_entries;

inline size_t memory_num_entries()
{
    return num_entries;
}

size_t memory_size()
{
    size_t size = 0;
    for(size_t i = 0; i < num_entries; i++)
        if(memory_map[i].type == MEM_USABLE)
            size += memory_map[i].length;
    return size;
}

bool memory_read_map()
{
    rm_regs_t regs;
    regs.ebx = 0;

    num_entries = 0;

    for(size_t i = 0; i < MAX_ENTRIES; i++)
    {
        regs.eax = 0xE820;
        regs.ecx = sizeof(mmap_entry_t);
        regs.edx = 0x534d4150;
        regs.edi = &memory_map[i];

        rm_int(0x15, &regs, &regs);
        num_entries++;

        if(regs.ebx == 0)
            return true;
    }

    return false;
}

size_t memory_lower_size()
{
    rm_regs_t regs;
    rm_int(0x12, &regs, &regs);
    return regs.eax * 1024;
}