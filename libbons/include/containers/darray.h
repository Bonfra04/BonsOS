#pragma once

#include <stddef.h>

void* __darray(size_t elem_size, size_t length);
#define darray(type, length) ((type*)__darray(sizeof(type), length))

void* __darray_append(void* darray, void* element);
#define darray_append(darray, element) ({ typeof(element) __tmp = element; darray = __darray_append(darray, &__tmp); })

void* __darray_remove(void* darray, size_t index);
#define darray_remove(darray, index) ({ darray = __darray_remove(darray, index); })

size_t __darray_find(void* darray, void* element);
#define darray_find(darray, element) ({ typeof(element) __tmp = element; __darray_find(darray, &__tmp); })

void darray_destroy(void* darray);

size_t darray_length(void* darray);
