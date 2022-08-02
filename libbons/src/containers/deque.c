#include <containers/deque.h>

#include <stdlib.h>

deque_t deque_create()
{
    deque_t queue;
    queue.head = NULL;
    queue.tail = NULL;
    queue.size = 0;
    return queue;
}

bool deque_push_front(deque_t* deque, void* data)
{
    deque_node_t* node = malloc(sizeof(deque_node_t));
    if (node == NULL)
        return false;

    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    if(deque->head != NULL)
    {
        deque->head->prev = node;
        node->next = deque->head;
    }
    deque->head = node;
    if(deque->tail == NULL)
        deque->tail = node;

    deque->size++;
    return true;
}

bool deque_push_back(deque_t* deque, void* data)
{
    deque_node_t* node = malloc(sizeof(deque_node_t));
    if (node == NULL)
        return false;

    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    if(deque->tail != NULL)
    {
        deque->tail->next = node;
        node->prev = deque->tail;
    }
    deque->tail = node;
    if(deque->head == NULL)
        deque->head = node;

    deque->size++;
    return true;
}

void* deque_pop_front(deque_t* deque)
{
    if(deque->head == NULL)
        return NULL;

    deque_node_t* node = deque->head;
    deque->head = node->next;
    if(deque->head == NULL)
        deque->tail = NULL;
    else
        deque->head->prev = NULL;

    void* data = node->data;
    free(node);
    deque->size--;
    return data;
}

void* deque_pop_back(deque_t* deque)
{
    if(deque->tail == NULL)
        return NULL;

    deque_node_t* node = deque->tail;
    deque->tail = node->prev;
    if(deque->tail == NULL)
        deque->head = NULL;
    else
        deque->tail->next = NULL;

    void* data = node->data;
    free(node);
    deque->size--;
    return data;
}

size_t deque_size(deque_t* deque)
{
    return deque->size;
}

void deque_clear(deque_t* deque)
{
    while(deque->head != NULL)
        deque_pop_front(deque);
}
