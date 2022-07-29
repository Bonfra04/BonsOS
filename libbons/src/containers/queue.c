#include <containers/queue.h>

#include <stddef.h>
#include <stdlib.h>

queue_t queue_new()
{
    queue_t queue;
    queue.head = NULL;
    queue.tail = NULL;
    queue.size = 0;
    return queue;
}

bool enqueue(queue_t* queue, void* value)
{
    queue_node_t* node = malloc(sizeof(queue_node_t));
    if(node == NULL)
        return false;

    node->value = value;
    node->next = NULL;
    if(queue->tail != NULL)
        queue->tail->next = node;
    queue->tail = node;
    if(queue->head == NULL)
        queue->head = node;

    queue->size++;
    return true;
}

void* dequeue(queue_t* queue)
{
    if(queue->head == NULL)
        return NULL;

    queue_node_t* head = queue->head;
    queue->head = head->next;
    if(queue->head == NULL)
        queue->tail = NULL;

    void* value = head->value;
    free(head);
    queue->size--;
    return value;
}

size_t queue_size(queue_t* queue)
{
    return queue->size;
}
