#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <syscall/syscall.h>
#include <schedule/scheduler.h>

uint64_t syscall_seek_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    size_t offset = (size_t)params->r9;
    fsys_set_position(file, offset);
    return 0;
}