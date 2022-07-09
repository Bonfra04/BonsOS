#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct iterator iterator_t;
struct iterator
{
    void* position;
    void (*next)(iterator_t* it);
    void (*prev)(iterator_t* it);
    void* structure_data;
};

/**
 * @brief creates an iterator
 * @param[in] next function called to get the next element
 * @param[in] prev function called to get the previous element
 * @param[in] position initial position of the iterator
 * @param[in] structure_data specific data used by the iterator functions
 * @return the iterator
 * @note structure_data is not owned by the iterator
 */
iterator_t iterator(void (*next)(iterator_t* it), void (*prev)(iterator_t* it), void* position, void* structure_data);

/**
 * @brief gets the element pointed by the iterator
 * @param[in] it iterator
 */
void* __iterator_get(const iterator_t* it);
#define iterator_get(it, type) (*(type*)__iterator_get(it))

/**
 * @brief advances an iterator by n elements
 * @param[in] it a pointer to the iterator
 * @param[in] n the number of elements to advance
 */
void iterator_advance(iterator_t* it, int64_t n);

/**
 * @brief calculates the number of elements between two iterators
 * @param[in] first first iterator
 * @param[in] last second iterator
 * @return the number of elements between the two iterators
 * @note the iterators must be of the same type and first must be before last
 */
uint64_t iterator_distance(const iterator_t* first, const iterator_t* last);

/**
 * @brief returns an iterator pointing to the element that would be pointed if advanced by -n elements
 * @param[in] it a pointer to the iterator
 * @param[in] n number of elements to go back
 * @return the iterator pointing to the desired element
 */
iterator_t iterator_prev(const iterator_t* it, uint64_t n);

/**
 * @brief returns an iterator pointing to the element that would be pointed if advanced by n elements
 * @param[in] it a pointer to the iterator
 * @param[in] n number of elements to go forward
 * @return the iterator pointing to the desired element 
 */
iterator_t iterator_next(const iterator_t* it, uint64_t n);
