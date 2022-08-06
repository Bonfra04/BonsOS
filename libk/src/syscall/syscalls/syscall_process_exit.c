#include <smp/scheduler.h>

void syscall_process_exit()
{
    scheduler_terminate_process();
}
