#include <stdio.h>
#include <stdarg.h>

#define STREAM_STDOUT 1
#define STREAM_STDIN 2
#define STREAM_STDERR 3

int vfprintf(FILE* stream, const char * format, va_list arg)
{
    char buf[1024];
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
    char buffer[1024];
    size_t pos = stream == (FILE*)STREAM_STDIN ? 0 : ftell(stream);
    size_t last = fread(&buffer, 1, 1024, stream);
    if(last == 0)
        return EOF;
    buffer[last] = '\0';
    int res = vsscanf(buffer, format, arg);
    if(stream != (FILE*)STREAM_STDIN)
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