#pragma once

#include <atomic/mutex.h>
#include <containers/queue.h>

typedef struct tsqueue_t
{
    queue_t queue;
    mutex_t operations_mutex;
    mutex_t blocking_mutex;
} tsqueue_t;

tsqueue_t tsqueue_create();
#define tsqueue() tsqueue_create()

bool tsqueue_enqueue(tsqueue_t* tsqueue, void* value);
void* tsqueue_dequeue(tsqueue_t* tsqueue);

size_t tsqueue_size(tsqueue_t* tsqueue);
void tsqueue_flush(tsqueue_t* tsqueue);

void tsqueue_wait(tsqueue_t* tsqueue);
