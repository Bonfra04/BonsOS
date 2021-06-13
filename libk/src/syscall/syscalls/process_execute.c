#include "../syscalls.h"
#include <executable/executable.h>

void syscall_process_execute(const syscall_parameter_t* params)
{
    const char* path = (const char*)params->r8;
    executable_format_t format = params->r9;
    run_executable(path, format);
}