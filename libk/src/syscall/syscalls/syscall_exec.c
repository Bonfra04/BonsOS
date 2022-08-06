#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>
#include <executable/executable.h>

void syscall_exec(const char* path, const char** argv)
{
    path = paging_get_ph(current_thread->proc->paging, path);

    executable_t* exec = executable_load(path);
    scheduler_replace_process(exec, NULL);
}
