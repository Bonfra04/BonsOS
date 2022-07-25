#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>
#include <stddef.h>

int syscall_read_file(int fd, char* buff, size_t count)
{
    buff = paging_get_ph(current_thread->proc->paging, buff);

    file_t* f = scheduler_get_resource(fd, RES_FILE);
    if(f == NULL)
        return -1;

    size_t res = fsys_read_file(f, buff, count);
    if(fsys_error(f))
        return -1;

    return res;
}
