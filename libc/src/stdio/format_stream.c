#include <stdio.h>
#include <stdarg.h>

int vfprintf(FILE* stream, const char * format, va_list arg)
{
    char buf[512];
    int res = vsprintf(buf, format, arg);
    return fwrite(&buf, 1, res, stream);
}

int fprintf(FILE* stream, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vfprintf(stream, format, args);
    va_end(args);
    return res;
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

int vfscanf(FILE* stream, const char* format, va_list arg)
{
    char buffer[513];
    size_t pos = ftell(stream);
    size_t last = fread(&buffer, 1, 512, stream);
    if(last == 0)
        return EOF;
    buffer[last] = '\0';
    int res = vsscanf(buffer, format, arg);
    fseek(stream, pos + res, SEEK_SET);
    return res;
}

int fscanf(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vfscanf(stream, format, args);
    va_end(args);
    return res;
}

int vscanf(const char* format, va_list arg)
{
    return vfscanf(stdin, format, arg);
}

int scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vscanf(format, args);
    va_end(args);
    return res;
}