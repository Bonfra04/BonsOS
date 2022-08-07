#include <fsys/fsys.h>
#include <smp/scheduler.h>
#include <memory/paging.h>

#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#define OPEN_READ FSYS_READ
#define OPEN_WRITE FSYS_WRITE
#define OPEN_APPEND FSYS_APPEND

int syscall_open_file(const char* path, int mode)
{
    path = paging_get_ph(current_thread->proc->paging, path);

    const char* fullpath;
    if(is_absolute(path))
    {
        fullpath = strdup(path);
        if(fullpath == NULL)
            return -1;
    }
    else
    {
        fullpath = malloc(strlen(path) + strlen(current_thread->proc->workdir) + 2);
        if(fullpath == NULL)
            return -1;
        strcpy(fullpath, current_thread->proc->workdir);
        strcat(fullpath, "/");
        strcat(fullpath, path);
    }
        
    file_t* f = malloc(sizeof(file_t));
    if(f == NULL)
    {
        free(fullpath);
        return -1;
    }

    *f = fsys_open_file(fullpath, mode);
    free(fullpath);
    if(fsys_error(f))
    {
        free(f);
        return -1;
    }
    
    return scheduler_alloc_resource(f, RES_FILE);
}
