#include <smp/scheduler.h>

void syscall_thread_exit()
{
    scheduler_terminate_thread();
}
