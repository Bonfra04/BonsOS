#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>
#include <executable/executable.h>

void syscall_exec(const char* path, const char** argv)
{
    path = paging_get_ph(current_thread->proc->paging, path);
    argv = paging_get_ph(current_thread->proc->paging, argv);

    executable_t* exec = executable_load(path);

    size_t num_args = 0;
    if(argv)
        for(; argv[num_args]; num_args++);

    char* args[num_args + 1];
    for(size_t i = 0; i < num_args; i++)
        args[i] = paging_get_ph(current_thread->proc->paging, argv[i]);
    args[num_args] = NULL;

    scheduler_replace_process(exec, args);
}
