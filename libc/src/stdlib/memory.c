#include <stdlib.h>
#ifdef KERNEL_BUILD
    #include <memory/heap.h>
#else
    #include <heap.h>
#endif

void* malloc(size_t size)
{
    return heap_malloc(size);
}

void free(void* ptr)
{
    heap_free(ptr);
}