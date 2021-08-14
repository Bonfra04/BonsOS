#include <smp/atomic.h>

inline void mutex_acquire(mutex_t* mutex)
{
    while (!__sync_bool_compare_and_swap(mutex, 0, 1));
	__sync_synchronize();
}

inline void mutext_release(mutex_t* mutex)
{
    __sync_synchronize();
	*mutex = 0;
}


void mutex_await(mutex_t* mutex)
{
    while (!__sync_bool_compare_and_swap(mutex, 0, 0));
	__sync_synchronize();
}