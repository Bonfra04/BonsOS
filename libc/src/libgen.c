#include <libgen.h>
#include <string.h>

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
