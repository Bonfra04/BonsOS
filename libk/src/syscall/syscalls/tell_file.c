#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <smp/scheduler.h>

uint64_t syscall_tell_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*)params->arg0);
    return fsys_get_position(file);
}