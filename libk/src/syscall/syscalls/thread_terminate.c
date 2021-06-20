#include "../syscalls.h"

#include <schedule/scheduler.h>

uint64_t syscall_thread_terminate(const syscall_parameter_t* params)
{
    (void)params;
    thread_terminate();
    return 0;
}