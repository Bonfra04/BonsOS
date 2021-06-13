#include "../syscalls.h"

#include <schedule/scheduler.h>

void syscall_thread_terminate(const syscall_parameter_t* params)
{
    thread_terminate();
}