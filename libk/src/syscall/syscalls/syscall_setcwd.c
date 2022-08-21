#include <smp/scheduler.h>
#include <fsys/fsys.h>

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "utils.h"

int syscall_setcwd(char* buff)
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

    char* simple = eval_path(path);
    free(path);

    free((char*)current_thread->proc->workdir);
    current_thread->proc->workdir = simple;

    return 0;
}
