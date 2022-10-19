#include <smp/scheduler.h>

void syscall_raise_signal(uint64_t tid, uint64_t signal)
{
    scheduler_raise_signal(tid, signal);
}
