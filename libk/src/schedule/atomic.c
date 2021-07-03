#include <schedule/atomic.h>

inline void atomic_start()
{
    asm volatile ("cli");
}

inline void atomic_end()
{
    asm volatile ("sti");
}

inline void mutex_acquire(mutex_t* mutex)
{
    while(!__sync_bool_compare_and_swap(mutex, 0, 1))
		scheduler_force_skip();
}

inline void mutext_release(mutex_t* mutex)
{
    *mutex = 0;
}
