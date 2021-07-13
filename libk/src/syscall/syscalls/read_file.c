#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/scheduler.h>
#include <schedule/atomic.h>

uint64_t syscall_read_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    void* buffer = vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*) params->r9);
    size_t length = params->r10;
    uint64_t result = fsys_read_file(file, buffer, length);
    return result;
}