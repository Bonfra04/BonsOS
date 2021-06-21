#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>

uint64_t syscall_file_close(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)params->r8;
    return fsys_close_file(file);
}