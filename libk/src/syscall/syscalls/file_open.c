#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <smp/scheduler.h>

uint64_t syscall_file_open(const syscall_parameter_t* params)
{
    const char* filename = (const char*)vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*)params->arg0);
    const char* mode = (const char*)vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*)params->arg1);
    file_t* out_file = (file_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*)params->arg2);
    *out_file = fsys_open_file(filename, mode);
    return 0;
}