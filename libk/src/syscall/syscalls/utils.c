#include "utils.h"
#include <smp/scheduler.h>

#include <stdlib.h>
#include <string.h>
#include <libgen.h>

char* expand_path(const char* path)
{
    const char* wd = current_thread->proc->workdir;
    char* fullpath = malloc(strlen(path) + strlen(wd) + 2);
    if(fullpath == NULL)
        return NULL;

    memset(fullpath, '\0', sizeof(fullpath));
    if(!is_absolute(path))
    {
        strcpy(fullpath, wd);
        strcat(fullpath, "/");
    }
    strcat(fullpath, path);

    return fullpath;
}