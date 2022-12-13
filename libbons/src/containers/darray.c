#include <containers/darray.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct darray_t
{
    size_t stride;
    size_t length;
} __attribute__((packed)) darray_t;

void* __darray(size_t elem_size, size_t length)
{
    darray_t* darray = malloc(sizeof(darray_t) + elem_size * length);

    darray->stride = elem_size;
    darray->length = length;

    return darray + 1;
}

void* __darray_append(void* darray, void* element)
{
    darray_t* desc = (darray_t*)darray - 1;
    size_t size = sizeof(darray_t) + desc->stride * ++desc->length;
    desc = realloc(desc, size);

    darray = desc + 1;
    memcpy((uint8_t*)darray + (desc->length - 1) * desc->stride, element, desc->stride);
    return darray;
}

void* __darray_remove(void* darray, size_t index)
{
    darray_t* desc = (darray_t*)darray - 1;
    memcpy((uint8_t*)darray + index * desc->stride, (uint8_t*)darray + (index + 1) * desc->stride, desc->stride * (desc->length - index - 1));

    size_t size = sizeof(darray_t) + desc->stride * (--desc->length);
    desc = realloc(desc, size);
    darray = desc + 1;
    return darray;
}

size_t __darray_find(void* darray, void* element)
{
    darray_t* desc = (darray_t*)darray - 1;
    for(size_t i = 0; i < desc->length; i++)
        if(memcmp((uint8_t*)darray + i * desc->stride, element, desc->stride) == 0)
            return i;

    return -1;
}

void darray_destroy(void* darray)
{
    free((darray_t*)darray - 1);
}

size_t darray_length(void* darray)
{
    darray_t* desc = (darray_t*)darray - 1;
    return desc->length;
}

void* __darray_pack(void* darray)
{
    darray_t* desc = (darray_t*)darray - 1;
    void* array = malloc(desc->stride * desc->length);
    memcpy(array, darray, desc->stride * desc->length);
    darray_destroy(darray);
    return array;
}
