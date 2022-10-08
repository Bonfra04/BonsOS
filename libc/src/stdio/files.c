#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <syscalls.h>

FILE* stdin;
FILE* stdout;
FILE* stderr;

void __files_init()
{
    stdin = calloc(sizeof(FILE), 1);
    stdin->fd = 0;
    stdin->flags = _IONBF;
    stdout = calloc(sizeof(FILE), 1);
    stdout->fd = 1;
    stdout->flags = _IONBF;
    stderr = calloc(sizeof(FILE), 1);
    stderr->fd = 2;
    stderr->flags = _IONBF;
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

    stream->fd = sys_open_file(filename, open_mode);
    if (stream->fd < 0)
    {
        free(stream);
        return NULL;
    }
    stream->flags = _IONBF;

    return stream;
}

FILE* freopen(const char* filename, const char* mode, FILE* stream)
{
    if(!filename || !mode)
        return NULL;

    int open_mode = mode[0] == 'r' ? OPEN_READ : mode[0] == 'w' ? OPEN_WRITE : mode[0] == 'a' ? OPEN_APPEND : -1;
    if(open_mode == -1)
        return NULL;

    if(!sys_close_file(stream->fd))
        return NULL;

    stream->fd = sys_open_file(filename, open_mode);
    if (stream->fd < 0)
    {
        free(stream);
        return NULL;
    }
    stream->flags = _IONBF;

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
    size_t res = sys_read_file(stream->fd, ptr, length);
    switch (res)
    {
    case 0:
        stream->flags |= FILE_EOF;
        return 0;
    case -1:
        stream->flags |= FILE_ERROR;
        return 0;
    default:
        return res;
    }
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

    switch (stream->flags & 0b11)
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
    size_t res = sys_write_file(stream->fd, ptr, length);
    if(res == -1)
    {
        stream->flags |= FILE_ERROR;
        res = 0;
    }
    return res;
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

    switch (stream->flags & 0b11)
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

int fputc(int character, FILE* stream)
{
    if(!stream)
        return EOF;

    return fwrite(&character, 1, 1, stream) == 1 ? character : EOF;
}

int putchar(int character)
{
    return fputc(character, stdout);
}

int fputs(const char* str, FILE* stream)
{
    if(!str || !stream)
        return EOF;

    size_t len = strlen(str);
    return fwrite(str, 1, len, stream) == len ? 1 : EOF;
}

int puts(const char* str)
{
    int res = fputs(str, stdout);
    if(res == EOF)
        return EOF;
    return fputc('\n', stdout);
}

int fgetc(FILE* stream)
{
    if(!stream)
        return EOF;

    int c;
    if(fread(&c, 1, 1, stream) == 0)
        return EOF;
    return c;
}

int getchar(void)
{
    return fgetc(stdin);
}

char* fgets(char* str, int num, FILE* stream)
{
    if(!str || !stream)
        return NULL;

    if(num == 0)
        return NULL;

    char* ptr = str;
    while(--num > 0)
    {
        int c = fgetc(stream);
        if(c == EOF)
            break;
        *ptr++ = c;
        if(c == '\n')
            break;
    }

    if(str == ptr)
        return NULL;

    *ptr = '\0';
    return str;
}

char* gets(char* str)
{
    if(!str)
        return NULL;

    char* ptr = str;
    for(;;)
    {
        int c = fgetc(stdin);
        if(c == EOF || c == '\n')
            break;
        *ptr++ = c;
    }

    if(str == ptr)
        return NULL;

    *ptr = '\0';
    return str;
}

int feof(FILE* stream)
{
    return stream->flags & FILE_EOF;
}

int ferror(FILE* stream)
{
    return stream->flags & FILE_ERROR;
}

void clearerr(FILE* stream)
{
    stream->flags &= ~FILE_ERROR;
    stream->flags &= ~FILE_EOF;
}

int remove(const char* filename)
{
    return sys_delete_file(filename) ? 0 : -1;
}

int fgetpos(FILE* stream, fpos_t* pos)
{
    size_t off = sys_tell_file(stream->fd);
    if(off == -1)
        return -1;
    
    *pos = off;
    return 0; 
}

int fsetpos(FILE* stream, const fpos_t* pos)
{
    return sys_seek_file(stream->fd, *pos) == true ? 0 : -1;
}

int fseek(FILE* stream, long offset, int origin)
{
    fpos_t base = -1;
    switch(origin)
    {
    case SEEK_CUR:
        if(fgetpos(stream, &base) != 0)
            return -1;
        break;
    case SEEK_SET:
        base = 0;
        break;
    case SEEK_END:
        if(fsetpos(stream, &base) != 0 || fgetpos(stream, &base) != 0)
            return -1;
        break;
    }
    size_t off = sys_seek_file(stream->fd, base + offset);
    return off == -1 ? -1 : 0;
}

long ftell(FILE* stream)
{
    return sys_tell_file(stream->fd);
}

void rewind(FILE* stream)
{
    fsetpos(stream, 0);
}
