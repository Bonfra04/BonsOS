#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <smp/scheduler.h>

uint64_t syscall_write_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)params->arg0;
    void* buffer = vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*) params->arg1);
    size_t length = params->arg2;
    size_t res = fsys_write_file(file, buffer, length);
    return res;
}