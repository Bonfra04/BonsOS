#include <stdio.h>
#include <stdarg.h>

int fprintf(FILE* stream, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vfprintf(stream, format, args);
    va_end(args);
    return res;
}

int vfprintf(FILE * stream, const char * format, va_list arg)
{
    char buf[512];
    int res = vsprintf(buf, format, arg);
    return fwrite(&buf, 1, res, stream);
}

int vprintf(const char * format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}

int printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);
    return res;
}