#include <queue.h>
#include <stdlib.h>
#include <string.h>

#define get_pqueue(queue) ((queue_data_t*)queue)

typedef struct node
{
    void* next;
    void* prev;
    void* value;
} __attribute__ ((packed)) node_t;

typedef struct queue_data
{
    node_t* front;
    node_t* back;
    size_t length;
    size_t elem_size;
} queue_data_t;

queue_t __queue_create(size_t elem_size)
{
    queue_data_t* queue = (queue_data_t*)malloc(sizeof(queue_data_t));
    memset(queue, 0, sizeof(queue_data_t));
    queue->elem_size = elem_size;
    return (queue_t)queue;
}

void queue_destroy(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    while(pqueue->length)
        queue_pop(queue);
    free(pqueue);
}

size_t queue_size(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    return pqueue->length;
}

bool queue_empty(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    return pqueue->length == 0;
}

void* queue_front(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    if(pqueue->length == 0)
        return 0;
    return pqueue->front->value;
}

void* queue_back(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    return pqueue->back->value;
}

void __queue_push(queue_t queue, const void* value_ptr)
{
    queue_data_t* pqueue = get_pqueue(queue);
    node_t* node = (node_t*)malloc(sizeof(node_t));
    if(node == NULL)
        return;

    if(pqueue->front == 0)
    {
        pqueue->back = node;
        pqueue->front = node;
        node->next = 0;
    }
    else
    {
        pqueue->back->prev = node;
        node->next = pqueue->back;
        pqueue->back = node;
    }
    node->prev = 0;
    node->value = malloc(pqueue->elem_size);
    memcpy(node->value, value_ptr, pqueue->elem_size);
    pqueue->length++;
}

void queue_pop(queue_t queue)
{
    queue_data_t* pqueue = get_pqueue(queue);
    if(pqueue->length == 0)
        return;
    
    node_t* node = pqueue->front;
    pqueue->front = pqueue->front->prev;
    pqueue->front->next = 0;
    free(node->value);
    free(node);
    pqueue->length--;
}
