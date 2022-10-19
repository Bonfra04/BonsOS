#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>
#include <executable/executable.h>

#include <libgen.h>
#include <string.h>
#include <stdlib.h>

uint64_t syscall_exec(const char* path, const char** argv, const char** env)
{
    path = paging_get_ph(current_thread->proc->paging, path);
    argv = paging_get_ph(current_thread->proc->paging, argv);
    env = paging_get_ph(current_thread->proc->paging, env);

    executable_t* exec = executable_load(path);
    if(exec == NULL)
        return;

    size_t num_args = 0;
    if(argv)
        for(; argv[num_args]; num_args++);
    const char* args[num_args + 1];
    for(size_t i = 0; i < num_args; i++)
        args[i] = paging_get_ph(current_thread->proc->paging, argv[i]);
    args[num_args] = NULL;

    size_t num_env = 0;
    if(env)
        for(; env[num_env]; num_env++);
    const char* envs[num_env + 1];
    for(size_t i = 0; i < num_env; i++)
        envs[i] = paging_get_ph(current_thread->proc->paging, env[i]);
    envs[num_env] = NULL;

    char* copy = strdup(path);
    process_t* proc = scheduler_run_executable(exec, current_thread->proc->workdir, argv ? args : NULL, env ? envs : NULL);
    free(copy);

    return proc->threads[0]->tid;
}
