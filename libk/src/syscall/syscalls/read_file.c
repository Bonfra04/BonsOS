#include "../syscalls.h"
#include <stdio.h>
#include <memory/virtual_memory_manager.h>

void syscall_read_file(const syscall_parameter_t* params)
{
    FILE* file = (FILE*)params->r8;
    void* buffer = vmm_translate_vaddr((void*) params->r9);
    size_t length = params->r10;
    fread(buffer, 1, length, file);
}