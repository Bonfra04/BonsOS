#include "../syscalls.h"
#include <executable/executable.h>
#include <memory/virtual_memory_manager.h>

uint64_t syscall_process_execute(const syscall_parameter_t* params)
{
    const char* path = (const char*)vmm_translate_vaddr((void*)params->r8);
    executable_format_t format = params->r9;
    int argc = (int)params->r10;
    char** argv = (char**)vmm_translate_vaddr((void*)params->r12);
    for(int i = 0; i < argc; i++)
        argv[i] = (char*)vmm_translate_vaddr((void*)argv[i]);
    return run_executable(path, argc, argv, format);
}