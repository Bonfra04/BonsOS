#include <executable/executable.h>
#include <stddef.h>
#include <schedule/scheduler.h>
#include <memory/page_frame_allocator.h>
#include <string.h>

typedef struct elf64_header
{
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__ ((packed)) elf64_header_t;

typedef enum segment_type
{
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR,
    PT_TLS,
} segment_type_t;

typedef struct program_header
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} __attribute__ ((packed)) program_header_t;

size_t run_elf_executable(void* address, size_t num_pages)
{
    elf64_header_t* header = (elf64_header_t*)address;
    program_header_t* phtable = (program_header_t*)(address + header->e_phoff); 
    
    uint64_t lower_addr = UINT64_MAX;
    uint64_t higher_addr = 0;
    for(size_t i = 0; i < header->e_phnum; i++)
        if(phtable[i].type == PT_LOAD)
        {
            if(phtable[i].vaddr < lower_addr)
                lower_addr = phtable[i].vaddr;
            if(phtable[i].vaddr + phtable[i].memsz > higher_addr)
                higher_addr = phtable[i].vaddr + phtable[i].memsz;
        }
    size_t size = higher_addr - lower_addr;
    size = size / pfa_page_size() + (size % pfa_page_size() != 0);

    void* new_addr = pfa_alloc_pages(size);
    memset(new_addr, 0, size * pfa_page_size());
    memcpy(new_addr, address + 0x1000, (num_pages - 1) * pfa_page_size());
    pfa_free_pages(address, num_pages);

    return create_process(new_addr, size);
}