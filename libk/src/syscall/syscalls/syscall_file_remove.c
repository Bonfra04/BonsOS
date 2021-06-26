#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>

uint64_t syscall_file_remove(const syscall_parameter_t* params)
{
    const char* filename = (const char*)params->r8;
    return fsys_delete_file(filename);
}