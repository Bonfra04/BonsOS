#include <memory/heap.h>

#include <stdlib.h>
#include <string.h>

void __memory_init()
{
    heap_init();
}

void* malloc(size_t size)
{
    return heap_malloc(size);
}

void free(void* ptr)
{
    heap_free(ptr);
}

void* calloc(size_t num, size_t size)
{
    void* ptr = malloc(num * size);
    if(ptr != NULL)
        memset(ptr, 0, num * size);
    return ptr;
}

void* realloc(void* ptr, size_t size)
{
    void* new_ptr = malloc(size);
    if(new_ptr != NULL)
    {
        memcpy(new_ptr, ptr, size);
        free(ptr);
    }
    return new_ptr;
}
