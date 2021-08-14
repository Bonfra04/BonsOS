#include "../syscalls.h"
#include <executable/executable.h>
#include <memory/virtual_memory_manager.h>
#include <smp/scheduler.h>

uint64_t syscall_process_execute(const syscall_parameter_t* params)
{
    const char* path = (const char*)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)params->arg0);
    executable_format_t format = params->arg1;
    int argc = (int)params->arg2;
    char** argv = (char**)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)params->arg3);
    for(int i = 0; i < argc; i++)
        argv[i] = (char*)vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, (void*)argv[i]);
    return run_executable(path, argc, argv, format);
}