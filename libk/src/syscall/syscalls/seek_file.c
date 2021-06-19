#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <syscall/syscall.h>

uint64_t syscall_seek_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)params->r8;
    size_t offset = (size_t)params->r9;
    fsys_set_position(file, offset);
    return 0;
}