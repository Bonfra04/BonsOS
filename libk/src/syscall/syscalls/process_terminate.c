#include "../syscalls.h"

#include <schedule/scheduler.h>

void syscall_process_terminate(const syscall_parameter_t* params)
{
    printf("CIAO");
    //process_terminate();
}