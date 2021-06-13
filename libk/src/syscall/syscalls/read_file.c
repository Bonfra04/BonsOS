#include "../syscalls.h"
#include <filesystem/fsys.h>

void syscall_read_file(const syscall_parameter_t* params)
{
    file_t* file = params->r8;
    void* buffer = params->r9;
    size_t length = params->r10;
    fsys_read_file(file, buffer, length);
}