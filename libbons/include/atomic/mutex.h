#pragma once

#include <stdatomic.h>

typedef uint64_t mutex_t;

/**
 * @brief waits for a mutex to be released, then locks it and return
 */
void mutex_acquire(mutex_t* mutex);

/**
 * @brief releases a mutex
 */
void mutex_release(mutex_t* mutex);
#define mutex_relase_return(mutex, value) ({ mutex_release(mutex); value; })
