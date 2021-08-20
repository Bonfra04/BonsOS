#pragma once

#define locked_read(addr) __atomic_load_n(addr, __ATOMIC_SEQ_CST)

#define locked_write(addr, val) __atomic_store_n(addr, val, __ATOMIC_SEQ_CST)

typedef volatile _Atomic unsigned char mutex_t;

/**
 * @brief waits for a mutex to be released, then locks it and return 
 */
void mutex_acquire(mutex_t* mutex);

/**
 * @brief releases a mutex
 */
void mutext_release(mutex_t* mutex);

void mutex_await(mutex_t* mutex);
