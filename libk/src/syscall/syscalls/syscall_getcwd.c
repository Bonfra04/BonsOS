#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stddef.h>
#include <string.h>

char* syscall_getcwd(char* buff, size_t size)
{
    buff =  paging_get_ph(current_thread->proc->paging, buff);

    const char* wd = current_thread->proc->workdir;
    if(strlen(wd) >= size)
        return NULL;

    strcpy(buff, wd);
    return buff;
}
