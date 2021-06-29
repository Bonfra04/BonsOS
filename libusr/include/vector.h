#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief creates a vector
 * @param[in] elem_size size in byte of each element
 * @return pointer to the array
 */
void* __vector_create(size_t elem_size);

/**
 * @brief destroys a vector
 * @param[in] vector vector to destroy
 */
void vector_destroy(void* vector);

/**
 * @brief returns the number of elements in a vector
 * @param[in] vector vector to get the size from
 * @return size
 */
size_t vector_size(void* vector);

/**
 * @brief returns the size of the storage space currently allocated for the vector
 * @param[in] vector vector to get the capacity from
 * @return capacity
 */
size_t vector_capacity(void* vector);

/**
 * @brief check if the vector is empty
 * @param[in] vector vector to check
 * @return emptiness
 */
bool vector_empty(void* vector);

/**
 * @brief resizes avector so that it contains n elements
 * @param[in] vector vector to resize
 * @param[in] length new length
 * @return new address for the vector
 */
void* __vector_resize(void* vector, size_t length);

/**
 * @brief sets the vector capacity to be at least enough to contain n elements
 * @param[in] vector vector to resize
 * @param[in] length new length
 * @return new address for the vector
 */
void* __vector_reserve(void* vector, size_t length);

/**
 * @brief push a value in the back of a vector
 * @param[in] vector vector to push the value on
 * @param[in] value_ptr a pointer to the value to push
 * @return new address of the vector
 */
void* __vector_push_back(void* vector, const void* value_ptr);

/**
 * @brief pop a value from the back of a vector
 * @param[in] vector vector to pop the value from
 * @return address of the popped element (the address may be invalidated on the next interaction)
 */
void* vector_pop_back(void* vector);

#define vector_create(type) __vector_create(sizeof(type))
#define vector_resize(vector, length) vector = __vector_resize(vector, length)
#define vector_reserve(vector, length) vector = __vector_reserve(vector, length)
#define vector_push_back(vector, value) { typeof(value) __tmp = value; vector = __vector_push_back(vector, &__tmp); }
