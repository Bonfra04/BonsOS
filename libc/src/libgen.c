#include <libgen.h>
#include <string.h>

char* dirname(char* path)
{
    static const char dot[] = ".";
    char *last_slash;

    last_slash = path != NULL ? (char*)strrchr(path, '/') : NULL;

    if (last_slash == path)
        last_slash++;
    else if (last_slash != NULL && last_slash[1] == '\0')
        last_slash = (char*)memchr(path, last_slash - path, '/');

    if (last_slash != NULL)
        last_slash[0] = '\0';
    else
        path = (char*)dot;

    return path;
}

const char* basename(const char* filename )
{
    const char *p = strrchr(filename, '/');
    return p ? p + 1 :filename;
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
