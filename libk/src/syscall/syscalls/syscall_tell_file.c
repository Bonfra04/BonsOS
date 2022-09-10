#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>
#include <stddef.h>

size_t syscall_tell_file(int fd)
{
    file_t* f = scheduler_get_resource(fd, RES_FILE);
    if(f == NULL)
        return -1;

    return fsys_get_position(f);
}
