#pragma once

#include <stdatomic.h>

typedef atomic_uchar mutex_t;

/**
 * @brief waits for a mutex to be released, then locks it and return
 */
void mutex_acquire(mutex_t* mutex);

/**
 * @brief releases a mutex
 */
void mutex_release(mutex_t* mutex);
