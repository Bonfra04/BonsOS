#include <smp/scheduler.h>
#include <fsys/fsys.h>

#include <stddef.h>
#include <string.h>

int syscall_setcwd(char* buff, size_t size)
{
    buff = paging_get_ph(current_thread->proc->paging, buff);

    char* path = expand_path(buff);
    if(path == NULL)
        return -1;
    
    if(!fsys_exists_dir(path))
    {
        free(path);
        return -1;
    }

    free(current_thread->proc->workdir);
    current_thread->proc->workdir = path;

    return 0;
}
