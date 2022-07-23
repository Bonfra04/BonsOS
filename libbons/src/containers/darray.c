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

void darray_destroy(void* darray)
{
    free((darray_t*)darray - 1);
}

size_t darray_length(void* darray)
{
    darray_t* desc = (darray_t*)darray - 1;
    return desc->length;
}
