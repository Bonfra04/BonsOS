#include <vector.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 24
#define DEFAULT_LEN 8

#define get_pvector(vector) (vector_data_t*)((uint8_t*)vector - HEADER_SIZE)
#define get_vector(pvector) ((uint8_t*)pvector + HEADER_SIZE)

typedef struct vector_data
{
    uint64_t capacity;
    uint64_t length;
    uint64_t stride;
} __attribute__ ((packed)) vector_data_t;

vector_t __vector_create(size_t elem_size)
{
    vector_data_t* vector = (vector_data_t*)malloc(HEADER_SIZE + DEFAULT_LEN * elem_size);
    vector->capacity = DEFAULT_LEN;
    vector->length = 0;
    vector->stride = elem_size;
    return (uint8_t*)vector + HEADER_SIZE;
}

void vector_destroy(vector_t vector)
{
    vector_data_t* pvector = get_pvector(vector);
    free(pvector);
}

size_t vector_size(vector_t vector)
{
    vector_data_t* pvector = get_pvector(vector);
    return pvector->length;
}

size_t vector_capacity(vector_t vector)
{
    vector_data_t* pvector = get_pvector(vector);
    return pvector->capacity;
}

bool vector_empty(vector_t vector)
{
    vector_data_t* pvector = get_pvector(vector);
    return pvector->length == 0;
}

vector_t __vector_resize(vector_t vector, size_t length)
{
    vector_data_t* pvector = get_pvector(vector);
    if(length <= pvector->capacity)
    {
        pvector->length = length;
        return get_vector(pvector);
    }
    vector_data_t* pvec_new = (vector_data_t*)malloc(HEADER_SIZE + length * pvector->stride);
    memcpy(pvec_new, pvector, pvector->length * pvector->stride);
    pvec_new->capacity = length;
    pvec_new->length = length;
    pvec_new->stride = pvector->stride;
    vector_destroy(pvector);
    return get_vector(pvec_new);
}

vector_t __vector_reserve(vector_t vector, size_t length)
{
    return __vector_resize(vector, length);
}

vector_t __vector_push_back(vector_t vector, const void* value_ptr)
{
    vector_data_t* pvector = get_pvector(vector);
    if(pvector->length == pvector->capacity)
    {
        vector_resize(vector, pvector->length * 2);
        pvector = get_pvector(vector);
    }
    memcpy((uint8_t*)vector + pvector->length * pvector->stride, value_ptr, pvector->stride);
    pvector->length++;
    return vector;
}

void* vector_pop_back(vector_t vector)
{
    vector_data_t* pvector = get_pvector(vector);
    if(pvector->length == 0)
        return 0;
    void* element = vector + pvector->length * pvector->stride;
    pvector->length--;
    return element;
}