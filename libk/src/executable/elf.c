#include <executable/elf.h>
#include <memory/pfa.h>

#include <stdlib.h>
#include <string.h>

typedef struct elf64_header
{
    uint8_t ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__ ((packed)) elf64_header_t;

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

bool elf_check(uint8_t* file_data)
{
    return file_data[0] == 0x7F && file_data[1] == 'E' && file_data[2] ==  'L' && file_data[3] == 'F';
}

executable_t* elf_load(uint8_t* file_data, size_t num_pages)
{
    elf64_header_t* header = (elf64_header_t*)file_data;
    program_header_t* phtable = (program_header_t*)(file_data + header->phoff);

    uint64_t lower_addr = UINT64_MAX;
    uint64_t higher_addr = 0;
    for(size_t i = 0; i < header->phnum; i++)
        if(phtable[i].type == PT_LOAD)
        {
            if(phtable[i].vaddr < lower_addr)
                lower_addr = phtable[i].vaddr;
            if(phtable[i].vaddr + phtable[i].memsz > higher_addr)
                higher_addr = phtable[i].vaddr + phtable[i].memsz;
        }

    size_t size = higher_addr - lower_addr;
    size = size / PFA_PAGE_SIZE + (size % PFA_PAGE_SIZE != 0);

    void* new_addr = pfa_calloc(size);
    memcpy(new_addr, file_data + 0x1000, (num_pages - 1) * PFA_PAGE_SIZE);
    pfa_free(file_data, num_pages);

    executable_t* exec = malloc(sizeof(executable_t));

    exec->format = EXEC_ELF;
    exec->base_address = new_addr;
    exec->num_pages = size;
    exec->entry_point = (uint8_t*)new_addr + header->entry - lower_addr;

    return exec;
}

void elf_unload(const executable_t* executable)
{
    pfa_free(executable->base_address, executable->num_pages);
    free(executable);
}
