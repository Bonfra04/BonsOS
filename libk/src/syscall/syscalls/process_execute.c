#include "../syscalls.h"
#include <executable/executable.h>

uint64_t syscall_process_execute(const syscall_parameter_t* params)
{
    const char* path = (const char*)params->r8;
    executable_format_t format = params->r9;
    return run_executable(path, 0, 0, format); // TODO arguments
}