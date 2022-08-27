#include <atomic/mutex.h>
#include <stdatomic.h>

#ifdef KERNEL_BUILD
#include <smp/scheduler.h>
void yield()
{
    scheduler_yield();
}
#else
#include <syscalls.h>
void yield()
{
    sys_sched_yield();
}
#endif

void mutex_acquire(mutex_t* mutex)
{
    uint32_t* next_ticket = (uint32_t*)mutex;
    uint32_t* serving_ticket = (uint32_t*)mutex + 1;

    uint32_t ticket = atomic_fetch_add(next_ticket, 1);
    while(atomic_load(serving_ticket) != ticket)
        yield();
}

void mutex_release(mutex_t* mutex)
{
    uint32_t* serving_ticket = (uint32_t*)mutex + 1;

    uint32_t current = atomic_load(serving_ticket);
    atomic_store(serving_ticket, current + 1);
}
