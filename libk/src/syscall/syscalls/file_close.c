#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>
#include <smp/scheduler.h>

uint64_t syscall_file_close(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)params->arg0);
    return fsys_close_file(file);
}