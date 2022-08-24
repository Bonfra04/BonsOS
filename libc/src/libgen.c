#include <libgen.h>
#include <string.h>
#include <stdlib.h>

#include <containers/darray.h>

char* dirname(char* path)
{
    static char dot[] = ".";

    if(!path)
        return NULL;

    char* p = (char*)strrchr(path, '/');
    if(!p)
        return dot;

    *(p + 1) = '\0';

    return path;
}

const char* basename(const char* filename)
{
    if(!filename)
        return NULL;
    const char* p = strrchr(filename, '/');
    return p ? p + 1 : filename;
}

bool is_absolute(const char* path)
{
    while(*path)
    {
        if(*path == '/')
            return false;
        else if(*path == ':')
            return true;
        path++;
    }
    return false;
}

char* eval_path(const char* path)
{
    if(!path)
        return NULL;

    char** stack = darray(char*, 0);

    char* copy = strdup(path);
    char* p = strtok(copy, "/");
    while(p != NULL)
    {
        if(strcmp(p, "..") == 0 && strlen(p) == 2)
            darray_remove(stack, darray_length(stack) - 1);
        else if(!(strcmp(p, ".") == 0 && strlen(p) == 1) && strlen(p) > 0)
            darray_append(stack, strdup(p));

        p = strtok(NULL, "/");
    }
    free(copy);

    size_t len = 0;
    for(size_t i = 0; i < darray_length(stack); i++)
        len += strlen(stack[i]) + 1;

    char* result = malloc(len + 1);
    result[0] = '\0';
    for(size_t i = 0; i < darray_length(stack); i++)
    {
        strcat(result, stack[i]);
        strcat(result, "/");
        free(stack[i]);
    }

    darray_destroy(stack);
    return result;
}
