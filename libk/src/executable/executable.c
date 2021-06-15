#include <executable/executable.h>
#include <memory/page_frame_allocator.h>

#include <stdio.h>
#include <stdbool.h>

extern void run_binary_executable(void* address, size_t num_pages);

void run_executable(const char* path, executable_format_t format)
{
    FILE* pFile = fopen(path, "r");
    if(!pFile)
        return;

    fseek(pFile, 0 , SEEK_END);
    size_t size = ftell(pFile);
    rewind(pFile);

    size_t num_pages = size / pfa_page_size() + (size % pfa_page_size() != 0);

    void* address = pfa_alloc_pages(num_pages);

    bool error = fread(address, 1, size, pFile) != size;
    fclose(pFile);
    if(error)
        return;

    switch (format)
    {
    case BINARY:
        run_binary_executable(address, num_pages);
        break;
    
    default:
        return;
    }
}