#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>

uint64_t syscall_file_open(const syscall_parameter_t* params)
{
    const char* filename = (const char*)params->r8;
    const char* mode = (const char*)params->r9;
    file_t* out_file = (file_t*)params->r10;
    *out_file = fsys_open_file(filename, mode);
    return 0;
}