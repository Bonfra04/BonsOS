#include <containers/linked_list.h>

#include <stdlib.h>
#include <string.h>

linked_list_t linked_list_new()
{
    return NULL;
}

void* __linked_list_get(linked_list_t list, size_t index)
{
    linked_list_node_t* node = list;
    while(node && index--)
        node = node->next;
    return node ? node->value : NULL;
}

linked_list_t __linked_list_append(linked_list_t list, size_t element_size, void* value)
{
    linked_list_node_t* node = malloc(sizeof(linked_list_node_t));
    node->value = malloc(element_size);
    memcpy(node->value, value, element_size);
    node->next = NULL;

    if (list == NULL)
    {
        list = node;
        return list;
    }

    linked_list_node_t* current = list;
    while (current->next != NULL)
        current = current->next;
    current->next = node;
    return list;
}

linked_list_t __linked_list_erase(linked_list_t list, size_t index)
{
    index -= 1;
    linked_list_node_t* node = list;
    while(node && index--)
        node = node->next;

    if(node == NULL)
        return list;

    linked_list_node_t* erased = node->next;
    node->next = erased->next;

    free(erased);

    return list;
}

size_t linked_list_size(linked_list_t list)
{
    size_t size = 0;
    linked_list_node_t* node = list;
    while (node != NULL)
    {
        size++;
        node = node->next;
    }
    return size;
}
