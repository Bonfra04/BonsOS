#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>
#include <stddef.h>

bool syscall_seek_file(int fd, size_t pos)
{
    file_t* f = scheduler_get_resource(fd, RES_FILE);
    if(f == NULL)
        return false;

    return fsys_set_position(f, pos);
}
