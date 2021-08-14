#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <syscall/syscall.h>
#include <smp/scheduler.h>

uint64_t syscall_seek_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)params->arg0);
    size_t offset = (size_t)params->arg1;
    fsys_set_position(file, offset);
    return 0;
}