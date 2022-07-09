#pragma once

#include <stddef.h>

typedef void* linked_list_t;

typedef struct linked_list_node linked_list_node_t;
typedef struct linked_list_node
{
    linked_list_node_t* next;
    void* value;
} linked_list_node_t;

linked_list_t linked_list_new();

void* __linked_list_get(linked_list_t list, size_t index);

#define linked_list_val(list, type, index) (*(type*)__linked_list_get(list, index))
#define linked_list_ptr(list, type, index) ((type*)__linked_list_get(list, index))

linked_list_t __linked_list_append(linked_list_t list, size_t element_size, void* value);

#define linked_list_append(list, type, value) { typeof(type) __val = value; list = __linked_list_append(list, sizeof(type), &__val); }

linked_list_t __linked_list_erase(linked_list_t list, size_t index);

#define linked_list_erase(list, index) { list = __linked_list_erase(list, index); }

size_t linked_list_size(linked_list_t list);
