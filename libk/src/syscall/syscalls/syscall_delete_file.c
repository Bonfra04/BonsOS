#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "utils.h"

bool syscall_delete_file(const char* path)
{
    path = paging_get_ph(current_thread->proc->paging, path);

    char* fullpath = expand_path(path);
    if(fullpath == NULL)
        return -1;

    bool res = fsys_delete_file(fullpath);
    free(fullpath);

    return res;
}
