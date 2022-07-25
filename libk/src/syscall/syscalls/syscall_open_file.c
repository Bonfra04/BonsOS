#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>

#define OPEN_READ FSYS_READ
#define OPEN_WRITE FSYS_WRITE
#define OPEN_APPEND FSYS_APPEND

int syscall_open_file(const char* path, int mode)
{
    path = paging_get_ph(current_thread->proc->paging, path);

    file_t* f = malloc(sizeof(file_t));
    if(f == NULL)
        return -1;

    *f = fsys_open_file(path, mode);
    if(fsys_error(f))
    {
        free(f);
        return -1;
    }

    return scheduler_alloc_resource(f, RES_FILE);
}
