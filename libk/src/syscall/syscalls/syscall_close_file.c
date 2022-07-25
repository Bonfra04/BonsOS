#include <fsys/fsys.h>
#include <smp/scheduler.h>

#include <stdlib.h>
#include <stdbool.h>

bool syscall_close_file(int fd)
{
    file_t* f = scheduler_get_resource(fd, RES_FILE);
    if(f == NULL)
        return false;

    bool res = fsys_close_file(f);
    if(res)
    {
        scheduler_free_resource(fd, RES_FILE);
        free(f);
    }
    return res;
}
