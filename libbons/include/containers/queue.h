#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct queue_node queue_node_t;
typedef struct queue_node
{
    void* value;
    queue_node_t* next;
} queue_node_t;

typedef struct {
    queue_node_t* head;
    queue_node_t* tail;
    size_t size;
} queue_t;

queue_t queue_new();
#define queue() queue_new()

bool enqueue(queue_t* queue, void* value);
void* dequeue(queue_t* queue);

size_t queue_size(queue_t* queue);
