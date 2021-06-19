#include "../syscalls.h"
#include <filesystem/fsys.h>
#include <memory/virtual_memory_manager.h>

uint64_t syscall_write_file(const syscall_parameter_t* params)
{
    file_t* file = (file_t*)params->r8;
    void* buffer = vmm_translate_vaddr((void*) params->r9);
    size_t length = params->r10;
    return fsys_write_file(file, buffer, length);
}