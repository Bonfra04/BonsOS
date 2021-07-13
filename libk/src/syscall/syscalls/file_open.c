#include "../syscalls.h"
#include <syscall/syscall.h>
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/scheduler.h>

uint64_t syscall_file_open(const syscall_parameter_t* params)
{
    const char* filename = (const char*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    const char* mode = (const char*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r9);
    file_t* out_file = (file_t*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r10);
    *out_file = fsys_open_file(filename, mode);
    return 0;
}