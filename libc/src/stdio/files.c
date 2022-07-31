#include <stdio.h>
#include <stdlib.h>

#include <syscalls.h>

FILE* stdin;
FILE* stdout;
FILE* stderr;

static void __attribute__((constructor)) __files_init()
{
    stdin = calloc(sizeof(FILE), 1);
    stdin->fd = 0;
    stdin->buffered = _IONBF;
    stdout = calloc(sizeof(FILE), 1);
    stdout->fd = 1;
    stdout->buffered = _IONBF;
    stderr = calloc(sizeof(FILE),1 );
    stderr->fd = 2;
    stderr->buffered = _IONBF;
}

FILE* fopen(const char* filename, const char* mode)
{
    // TODO: support all modifiers

    if(!filename || !mode)
        return NULL;

    int open_mode = mode[0] == 'r' ? OPEN_READ : mode[0] == 'w' ? OPEN_WRITE : mode[0] == 'a' ? OPEN_APPEND : -1;
    if(open_mode == -1)
        return NULL;

    FILE* stream = calloc(sizeof(FILE), 1);
    if(stream == NULL)
        return NULL;

    stream->fd = sys_open_file(filename, mode[0]);
    if (stream->fd < 0)
    {
        free(stream);
        return NULL;
    }
    stream->buffered = _IONBF;

    return stream;
}

int fclose(FILE* stream)
{
    if(!stream)
        return EOF;

    if(sys_close_file(stream->fd))
    {
        free(stream);
        return 0;
    }

    return EOF;
}

static size_t fread_unbuffered(void* ptr, size_t length, FILE* stream)
{
    return sys_read_file(stream->fd, ptr, length);
}

static size_t fread_linebuffered(void* ptr, size_t length, FILE* stream)
{
    (void)ptr; (void)length; (void)stream;
    return 0; // TODO: implement
}

static size_t fread_buffered(void* ptr, size_t length, FILE* stream)
{
    (void)ptr; (void)length; (void)stream;
    return 0; // TODO: implement
}

size_t fread(void* ptr, size_t size, size_t count, FILE *stream)
{
    if(!ptr || !stream)
        return 0;

    if(size == 0 || count == 0)
        return 0;

    switch (stream->buffered)
    {
    case _IONBF:
        return fread_unbuffered(ptr, size * count, stream);
    case _IOLBF:
        return fread_linebuffered(ptr, size * count, stream);
    case _IOFBF:
        return fread_buffered(ptr, size * count, stream);
    }

    return 0;
}

static size_t fwrite_unbuffered(const void* ptr, size_t length, FILE* stream)
{
    return sys_write_file(stream->fd, ptr, length);
}

static size_t fwrite_linebuffered(const void* ptr, size_t length, FILE* stream)
{
    (void)ptr; (void)length; (void)stream;
    return 0; // TODO: implement
}

static size_t fwrite_buffered(const void* ptr, size_t length, FILE* stream)
{
    (void)ptr; (void)length; (void)stream;
    return 0; // TODO: implement
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE *stream)
{
    if(!ptr || !stream)
        return 0;

    if(size == 0 || count == 0)
        return 0;

    switch (stream->buffered)
    {
    case _IONBF:
        return fwrite_unbuffered(ptr, size * count, stream);
    case _IOLBF:
        return fwrite_linebuffered(ptr, size * count, stream);
    case _IOFBF:
        return fwrite_buffered(ptr, size * count, stream);
    }

    return 0;
}
