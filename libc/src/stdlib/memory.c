#include <stdlib.h>
#include <memory/heap.h>

void* malloc(size_t size)
{
    return heap_malloc(size);
}

void free(void* ptr)
{
    heap_free(ptr);
}