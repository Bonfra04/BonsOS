#pragma once

#include <containers/iterator.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct vector
{
    uint64_t capacity;
    uint64_t length;
    uint64_t stride;
    void* data;
} vector_t;

/**
 * @brief creates a vector
 * @param[in] elem_size size in byte of each element
 * @return the vector
 */
vector_t __vector_create(size_t elem_size);
#define vector(type) __vector_create(sizeof(type))

/**
 * @brief destroys a vector
 * @param[in] vector vector to destroy
 */
void vector_destroy(vector_t* vector);

/**
 * @brief returns an iterator to the beginning of the vector
 * @param[in] vector vector to iterate
 * @return iterator to the beginning of the vector
 */
iterator_t vector_begin(const vector_t* vector);

/**
 * @brief returns an iterator to the end of the vector
 * @param[in] vector vector to iterate
 * @return iterator to the end of the vector
 */
iterator_t vector_end(const vector_t* vector);

/**
 * @brief returns the number of elements in a vector
 * @param[in] vector vector to get the size from
 * @return number of elements
 */
size_t vector_size(const vector_t* vector);

/**
 * @brief resizes a vector so that it contains n elements
 * @param[in] vector vector to resize
 * @param[in] length new length
 */
void vector_resize(vector_t* vector, size_t length);

/**
 * @brief returns the size of the storage space currently allocated for the vector
 * @param[in] vector vector to get the capacity from
 * @return capacity
 */
size_t vector_capacity(const vector_t* vector);

/**
 * @brief sets the vector capacity to be at least enough to contain n elements
 * @param[in] vector vector to resize
 * @param[in] length new length
 */
void vector_reserve(vector_t* vector, size_t length);

/**
 * @brief accesses the i-th element of the vector
 * @param[in] vector vector to get the element from
 * @param[in] i index of the element
 * @return a pointer to the the i-th element of the vector
 */
void* __vector_at(const vector_t* vector, size_t index);
#define vector_at(vector, type, index) ((type*)__vector_at(vector, index))

#define vector_front(vector, type) vector_at(vector, type, 0)
#define vector_back(vector, type) vector_at(vector, type, vector_size(vector) - 1)

/**
 * @brief checks if the vector is empty
 * @param[in] vector vector to check
 * @return true if the wstring is empty, false otherwise
 */
bool vector_empty(const vector_t* vector);

/**
 * @brief assigns new contents to the vector, replacing its current contents, and modifying its size accordingly
 * @param[in] vector vector to assign to
 * @param[in] from iterator to the first element in the sequence to assign
 * @param[in] to iterator to the end of the sequence to assign
 */
void vector_assign_range(vector_t* vector, iterator_t from, iterator_t to);

/**
 * @brief assigns new contents to the vector, replacing its current contents, and modifying its size accordingly
 * @param[in] vector vector to assign to
 * @param[in] n number of elements to assign
 * @param[in] val_ptr value to assign
 */
void __vector_assign_fill(vector_t* vector, size_t n, const void* val_ptr);
#define vector_assign_fill(vector, n, value) { typeof(value) __tmp = value; __vector_assign_fill(vector, n, &__tmp); }

/**
 * @brief push a value in the back of a vector
 * @param[in] vector vector to push the value on
 * @param[in] value_ptr a pointer to the value to push
 */
void __vector_push_back(vector_t* vector, const void* value_ptr);
#define vector_push_back(vector, value) { typeof(value) __tmp = value; __vector_push_back(vector, &__tmp); }

/**
 * @brief pop a value from the back of a vector
 * @param[in] vector vector to pop the value from
 */
void vector_pop_back(vector_t* vector);

/**
 * @brief extends the vector inserting a new element at the specified position
 * @param[in] vector vector to extend
 * @param[in] position iterator to the position to insert the new element
 * @param[in] val_ptr a pointer to the value to insert
 */
void __vector_insert_single(vector_t* vector, iterator_t position, const void* val_ptr);
#define vector_insert_single(vector, position, value) { typeof(value) __tmp = value; __vector_insert_single(vector, position, &__tmp); }

/**
 * @brief extends the vector inserting n new elements at the specified position
 * @param[in] vector vector to extend
 * @param[in] position iterator to the position to insert the new element
 * @param[in] n number of elements to insert
 * @param[in] val_ptr a pointer to the value to insert
 */
void __vector_insert_fill(vector_t* vector, iterator_t position, size_t n, const void* val_ptr);
#define vector_insert_fill(vector, position, n, value) { typeof(value) __tmp = value; __vector_insert_fill(vector, position, n, &__tmp); }

/**
 * @brief extends the vector inserting a range of elements at the specified position
 * @param[in] vector vector to extend
 * @param[in] position iterator to the position to insert the new element
 * @param[in] first iterator to the first element in the range to insert
 * @param[in] last iterator to the end of the range to insert
 */
void vector_insert_range(vector_t* vector, iterator_t position, iterator_t first, iterator_t last);

/**
 * @brief removes an element from a vector
 * @param[in] vector vector to remove the element from
 * @param[in] position iterator to the element to remove
 */
void vector_erase(vector_t* vector, iterator_t position);

/**
 * @brief removes a range of elements from a vector
 * @param[in] vector vector to remove the elements from
 * @param[in] first iterator to the first element in the range to remove
 * @param[in] last iterator to the end of the range to remove
 */
void vector_erase_range(vector_t* vector, iterator_t first, iterator_t last);

/**
 * @brief removes all elements from the vector
 * @param[in] vector vector to clear
 */
void vector_clear(vector_t* vector);
