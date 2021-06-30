#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void* queue_t;

/**
 * @brief creates a queue
 * @param[in] elem_size size in byte of each element
 * @return the queue object
 */
queue_t __queue_create(size_t elem_size);

/**
 * @brief destroys a queue
 * @param[in] queue queue to destroy
 */
void queue_destroy(queue_t queue);

/**
 * @brief returns the number of elements in a queue
 * @param[in] queue queue to get the size from
 * @return size
 */
size_t queue_size(queue_t queue);

/**
 * @brief check if the queue is empty
 * @param[in] queue queue to check
 * @return emptiness
 */
bool queue_empty(queue_t queue);

/**
 * @brief returns a pointer to the front element of the queue
 * @param[in] queue queue to interrogate
 * @return pointer to the element
 */
void* queue_front(queue_t queue);

/**
 * @brief returns a pointer to the back element of the queue
 * @param[in] queue queue to interrogate
 * @return pointer to the element
 */
void* queue_back(queue_t queue);

/**
 * @brief push a value a queue
 * @param[in] queue queue to push the value on
 * @param[in] value_ptr a pointer to the value to push
 */
void __queue_push(queue_t queue, const void* value_ptr);

/**
 * @brief pop a value from a queue
 * @param[in] queue queue to pop the value from
 */
void queue_pop(queue_t queue);

#define queue_create(type) __queue_create(sizeof(type))
#define queue_push(queue, value) { typeof(value) __tmp = value; __queue_push(queue, &__tmp); }
