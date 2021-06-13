#include "../syscalls.h"
#include <stdio.h>
#include <memory/virtual_memory_manager.h>

void syscall_write_file(const syscall_parameter_t* params)
{
    FILE* file = params->r8;
    void* buffer = vmm_translate_vaddr((void*) params->r9);
    size_t length = params->r10;
    fwrite(buffer, 1, length, file);
}