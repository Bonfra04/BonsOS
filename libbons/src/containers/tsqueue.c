#include <containers/tsqueue.h>

tsqueue_t tsqueue_create()
{
    tsqueue_t queue;
    queue.queue = queue_new();
    queue.operations_mutex = 0;
    queue.blocking_mutex = 0;
    return queue;
}

bool tsqueue_enqueue(tsqueue_t* tsqueue, void* value)
{
    mutex_acquire(&tsqueue->operations_mutex);
    bool success = enqueue(&tsqueue->queue, value);
    mutex_release(&tsqueue->operations_mutex);
    mutex_release(&tsqueue->blocking_mutex);
    return success;
}

void* tsqueue_dequeue(tsqueue_t* tsqueue)
{
    mutex_acquire(&tsqueue->operations_mutex);
    void* value = dequeue(&tsqueue->queue);
    mutex_release(&tsqueue->operations_mutex);
    return value;
}

void tsqueue_wait(tsqueue_t* tsqueue)
{
    while(queue_size(&tsqueue->queue) == 0)
    {
        mutex_acquire(&tsqueue->blocking_mutex); // acquired
        mutex_acquire(&tsqueue->blocking_mutex); // cannot reacquire until a release inside enqueue happens
        mutex_release(&tsqueue->blocking_mutex);
    }
}