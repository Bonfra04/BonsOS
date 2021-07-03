#pragma once

typedef volatile int mutex_t;

/**
 * @brief starts an atomic operation
 */
void atomic_start();

/**
 * @brief ends an atomic operation
 */
void atomic_end();

/**
 * waits for a mutex to be released, then locks it and return 
 */
void mutex_acquire(mutex_t* mutex);

/**
 * releases a mutex
 */
void mutext_release(mutex_t* mutex);
