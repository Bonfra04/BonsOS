#include <smp/scheduler.h>

void syscall_sched_yield()
{
    scheduler_yield();
}
