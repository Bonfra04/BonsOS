#include "../syscalls.h"

#include <schedule/scheduler.h>

uint64_t syscall_process_terminate(const syscall_parameter_t* params)
{
    (void)params;
    process_terminate();
    return 0;
}