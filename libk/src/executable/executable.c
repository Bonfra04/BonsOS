#include <executable/executable.h>
#include <memory/page_frame_allocator.h>

#include <stdio.h>
#include <stdbool.h>

extern size_t run_binary_executable(void* address, size_t num_pages);
extern size_t run_elf_executable(void* address, size_t num_pages);

size_t run_executable(const char* path, executable_format_t format)
{
    FILE* pFile = fopen(path, "r");
    if(!pFile)
        return 0;

    fseek(pFile, 0 , SEEK_END);
    size_t size = ftell(pFile);
    rewind(pFile);

    size_t num_pages = size / pfa_page_size() + (size % pfa_page_size() != 0);

    void* address = pfa_alloc_pages(num_pages);

    bool error = fread(address, 1, size, pFile) != size;
    fclose(pFile);
    if(error)
        return 0;

    switch (format)
    {
    case BINARY:
        return run_binary_executable(address, num_pages);
    case ELF:
        return run_elf_executable(address, num_pages);
    
    default:
        return 0;
    }
}