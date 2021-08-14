#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <smp/scheduler.h>

uint64_t syscall_read_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)params->arg0);
    void* buffer = vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*) params->arg1);
    size_t length = params->arg2;
    uint64_t result = fsys_read_file(file, buffer, length);
    return result;
}