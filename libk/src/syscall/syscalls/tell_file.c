#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/scheduler.h>

uint64_t syscall_tell_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    return fsys_get_position(file);
}