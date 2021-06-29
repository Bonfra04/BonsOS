#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief creates a queue
 * @param[in] elem_size size in byte of each element
 * @return pointer to the array
 */
void* __queue_create(size_t elem_size);

/**
 * @brief destroys a queue
 * @param[in] queue queue to destroy
 */
void queue_destroy(void* queue);

/**
 * @brief returns the number of elements in a queue
 * @param[in] queue queue to get the size from
 * @return size
 */
size_t queue_size(void* queue);

/**
 * @brief check if the queue is empty
 * @param[in] queue queue to check
 * @return emptiness
 */
bool queue_empty(void* queue);

/**
 * @brief returns a pointer to the front element of the queue
 * @param[in] queue queue to interrogate
 * @return pointer to the element
 */
void* queue_front(void* queue);

/**
 * @brief returns a pointer to the back element of the queue
 * @param[in] queue queue to interrogate
 * @return pointer to the element
 */
void* queue_back(void* queue);

/**
 * @brief push a value a queue
 * @param[in] queue queue to push the value on
 * @param[in] value_ptr a pointer to the value to push
 * @return new address of the queue
 */
void __queue_push(void* queue, const void* value_ptr);

/**
 * @brief pop a value from a queue
 * @param[in] queue queue to pop the value from
 */
void queue_pop(void* queue);

#define queue_create(type) __queue_create(sizeof(type))
#define queue_push(queue, value) { typeof(value) __tmp = value; __queue_push(queue, &__tmp); }
