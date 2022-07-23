#include <containers/vector.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_LEN 16

static void vector_iterator_next(iterator_t* it)
{
    vector_t* vector = it->structure_data;
    it->position = (uint8_t*)it->position + vector->stride;
}

static void vector_iterator_prev(iterator_t* it)
{
    vector_t* vector = it->structure_data;
    it->position = (uint8_t*)it->position - vector->stride;
}

vector_t __vector_create(size_t elem_size)
{
    vector_t vector = {
        .stride = elem_size,
        .length = 0,
        .capacity = DEFAULT_LEN,
        .data = malloc(elem_size * DEFAULT_LEN)
    };
    return vector;
}

void vector_destroy(vector_t* vector)
{
    free(vector);
}

iterator_t vector_begin(const vector_t* vector)
{
    void* ptr = vector->data;
    return iterator(vector_iterator_next, vector_iterator_prev, ptr, (void*)vector);
}

iterator_t vector_end(const vector_t* vector)
{
    void* ptr = (uint8_t*)vector->data + vector->length * vector->stride;
    return iterator(vector_iterator_next, vector_iterator_prev, ptr, (void*)vector);
}

size_t vector_size(const vector_t* vector)
{
    return vector->length;
}

void vector_resize(vector_t* vector, size_t length)
{
    vector_reserve(vector, length);
    vector->length = length;
}

size_t vector_capacity(const vector_t* vector)
{
    return vector->capacity;
}

void vector_reserve(vector_t* vector, size_t length)
{
    if(length <= vector->capacity)
        return;
    vector->data = realloc(vector->data, vector->stride * length);
    vector->capacity = length;
}

void* __vector_at(const vector_t* vector, size_t index)
{
    assert(index < vector->length);
    return (uint8_t*)vector->data + index * vector->stride;
}

bool vector_empty(const vector_t* vector)
{
    return vector->length == 0;
}

void vector_assign_range(vector_t* vector, iterator_t from, iterator_t to)
{
    vector_clear(vector);

    while(iterator_distance(&from, &to) > 0)
    {
        __vector_push_back(vector, __iterator_get(&from));
        iterator_advance(&from, 1);
    }
}

void __vector_assign_fill(vector_t* vector, size_t n, const void* val_ptr)
{
    vector_clear(vector);

    while(n--)
        __vector_push_back(vector, val_ptr);
}

void __vector_push_back(vector_t* vector, const void* value_ptr)
{
    if(vector->length == vector->capacity)
        vector_reserve(vector, vector->length * 2);

    memcpy((uint8_t*)vector->data + vector->length * vector->stride, value_ptr, vector->stride);
    vector->length++;
}

void vector_pop_back(vector_t* vector)
{
    assert(vector->length > 0);
    vector->length--;
}

void __vector_insert_single(vector_t* vector, iterator_t position, const void* val_ptr)
{
    assert(position.structure_data == vector);

    iterator_t end = vector_end(vector);
    
    size_t shift_size = iterator_distance(&position, &end) * vector->stride;
    ptrdiff_t shift_from = (uint8_t*)position.position - (uint8_t*)vector->data;
    ptrdiff_t shift_to = shift_from + vector->stride;

    vector_resize(vector, vector->length + 1);

    memmove((uint8_t*)vector->data + shift_to, (uint8_t*)vector->data + shift_from, shift_size);
    memcpy((uint8_t*)vector->data + shift_from, val_ptr, vector->stride);
}

void __vector_insert_fill(vector_t* vector, iterator_t position, size_t n, const void* val_ptr)
{
    assert(position.structure_data == vector);

    iterator_t end = vector_end(vector);

    size_t shift_size = iterator_distance(&position, &end) * vector->stride;
    ptrdiff_t shift_from = (uint8_t*)position.position - (uint8_t*)vector->data;
    ptrdiff_t shift_to = shift_from + vector->stride * n;

    vector_resize(vector, vector->length + n);

    memmove((uint8_t*)vector->data + shift_to, (uint8_t*)vector->data + shift_from, shift_size);
    while(n--)
    {
        void* data_offset = (uint8_t*)vector->data + shift_from + vector->stride * n;
        memcpy(data_offset, val_ptr, vector->stride);
    }
}

void vector_insert_range(vector_t* vector, iterator_t position, iterator_t first, iterator_t last)
{
    assert(position.structure_data == vector);

    size_t n = iterator_distance(&first, &last);

    iterator_t end = vector_end(vector);

    size_t shift_size = iterator_distance(&position, &end) * vector->stride;
    ptrdiff_t shift_from = (uint8_t*)position.position - (uint8_t*)vector->data;
    ptrdiff_t shift_to = shift_from + vector->stride * n;

    vector_resize(vector, vector->length + n);

    memmove((uint8_t*)vector->data + shift_to, (uint8_t*)vector->data + shift_from, shift_size);
    for(uint64_t i = 0; i < n; i++)
    {
        void* data_offset = (uint8_t*)vector->data + shift_from + vector->stride * i;
        memcpy(data_offset, __iterator_get(&first), vector->stride);
        iterator_advance(&first, 1);
    }
}

void vector_erase(vector_t* vector, iterator_t position)
{
    assert(position.structure_data == vector);

    iterator_t end = vector_end(vector);
    size_t n = iterator_distance(&position, &end);
    memmove(position.position, position.position + vector->stride, n * vector->stride);
    vector_resize(vector, vector->length - 1);
}

void vector_erase_range(vector_t* vector, iterator_t first, iterator_t last)
{
    assert(first.structure_data == vector);
    assert(last.structure_data == vector);

    size_t n = iterator_distance(&first, &last);
    iterator_t end = vector_end(vector);
    size_t shift_size = iterator_distance(&last, &end) * vector->stride;

    memmove(first.position, last.position, shift_size);
    vector_resize(vector, vector->length - n);
}

void vector_clear(vector_t* vector)
{
    vector->length = 0;
}