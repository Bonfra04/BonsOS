#include <stdio.h>
#include <string.h>
#include <filesystem/fsys.h>
#include <filesystem/ttyfs.h>
#include <filesystem/kbfs.h>
#include "../syscall/syscall_api.h"

FILE* stdout;
FILE* stderr;
FILE* stdin;

static file_t opened_files[FOPEN_MAX];
static FILE object_files[FOPEN_MAX];

static char buffers[FOPEN_MAX][BUFSIZ];

#define STREAM_STDOUT 1
#define STREAM_STDIN 2
#define STREAM_STDERR 3

#define MODE_READ       0b000001
#define MODE_WRITE      0b000010
#define MODE_APPEND     0b000100
#define MODE_UPDATE     0b001000
#define STATUS_READING  0b010000
#define STATUS_WRITING  0b100000

void stdio_stream_init()
{
    stdout = (FILE*)STREAM_STDOUT;
    stdin = (FILE*)STREAM_STDIN;
    stderr = (FILE*)STREAM_STDERR;

    for(int i = 0; i < FOPEN_MAX; i++)
    {
        opened_files[i].flags = FS_INVALID;
        object_files[i].buffer = (void*)&(buffers[i]);
        object_files[i].buffer_size = BUFSIZ;
        object_files[i].buffered = -1;
    }
}

static file_t* get_file(FILE* stream)
{
    int index = -1;
    for(int i = 0; i < FOPEN_MAX; i++)
        if(stream->address == object_files[i].address)
        {
            index = i;
            break;
        }
    if(index == -1)
        return NULL;
    return &(opened_files[index]);
}

static size_t fread_unbuffered(const void* ptr, size_t length, FILE* stream)
{
    size_t res = SYS_FREAD(get_file(stream), (void*)ptr, length);
    if(res == FS_EOF)
        return EOF;
    return res;
}

static size_t fread_linebuffered(const void* ptr, size_t length, FILE* stream)
{
    file_t* file = get_file(stream);

    // If nothing is buffered, buffer something
    if(stream->buffered == -1)
    {
        stream->buffered = SYS_FTELL(file);
        char c;
        size_t adv = 0;
        do {
            SYS_FREAD(file, &c, 1);
            ((char*)stream->buffer)[adv++] = c;
            if(file->error || file->eof)
                break;
        } while(adv < stream->buffer_size && c != '\n');
        if(file->error)
            return 0;
        SYS_FSEEK(file, 0);
    }

    size_t pos = SYS_FTELL(file);
    size_t bred = stream->buffered;
    size_t bsiz = stream->buffer_size;

    // amount of data already present in the buffer
    size_t already_buffred = 0;

    // to-read part is inside the buffer
    if(pos >= bred && (pos + length) <= (bred + bsiz))
    {
        memcpy((void*)ptr, (uint8_t*)stream->buffer + (pos - bred), length);
        SYS_FSEEK(file, pos + length);
        return length;
    }
    // to-read is below and inside the buffer
    else if(pos < bred && bred <= (pos + length) && (bred + bsiz) >= (pos + length))
    {
        already_buffred = pos + length - bred;
        size_t offset = length - (bred - pos);
        memcpy((uint8_t*)ptr + offset, stream->buffer, pos + length - bred);
        length -= pos + length - bred;

    }
    // to-read is inside and over the buffer
    else if(bred <= pos && pos <= (bred + bsiz) && (bred + bsiz) <= (pos + length))
    {
        size_t size = bred + bsiz - pos;
        already_buffred = size;
        memcpy((void*)ptr, (uint8_t*)stream->buffer + (pos - bred), size);
        length -= size;
        ptr = (uint8_t*)ptr + size;
        SYS_FSEEK(file, stream->buffered + size);
    }

    // read the data that isn't buffered
    size_t advance = 0;
    while (length > 0)
    {
        stream->buffered = SYS_FTELL(file);
        size_t res = 0;
        {
            char c;
            do {
                SYS_FREAD(file, &c, 1);
                ((char*)stream->buffer)[res++] = c;
                if(file->error || file->eof)
                    break;
            } while(res < stream->buffer_size && c != '\n');
        }

        if(file->error)
            break;
        
        size_t readable = length < stream->buffer_size ? length : stream->buffer_size;
        readable = res < readable ? res : readable;
        memcpy((uint8_t*)ptr + advance, stream->buffer, readable);
        length -= readable;
        advance += readable;
        SYS_FSEEK(file, stream->buffered + readable);

        if(file->eof)
            break;
    }

    return advance + already_buffred;

    return 0;
}

static size_t fread_fullybuffered(const void* ptr, size_t length, FILE* stream)
{
    file_t* file = get_file(stream);

    // If nothing is buffered, buffer something
    if(stream->buffered == -1)
    {
        stream->buffered = SYS_FTELL(file);
        SYS_FREAD(file, stream->buffer, stream->buffer_size);
        if(file->error)
            return 0;
        SYS_FSEEK(file, 0);
    }

    size_t pos = SYS_FTELL(file);
    size_t bred = stream->buffered;
    size_t bsiz = stream->buffer_size;

    // amount of data already present in the buffer
    size_t already_buffred = 0;

    // to-read part is inside the buffer
    if(pos >= bred && (pos + length) <= (bred + bsiz))
    {
        memcpy((void*)ptr, (uint8_t*)stream->buffer + (pos - bred), length);
        SYS_FSEEK(file, pos + length);
        return length;
    }
    // to-read is below and inside the buffer
    else if(pos < bred && bred <= (pos + length) && (bred + bsiz) >= (pos + length))
    {
        already_buffred = pos + length - bred;
        size_t offset = length - (bred - pos);
        memcpy((uint8_t*)ptr + offset, stream->buffer, pos + length - bred);
        length -= pos + length - bred;

    }
    // to-read is inside and over the buffer
    else if(bred <= pos && pos <= (bred + bsiz) && (bred + bsiz) <= (pos + length))
    {
        size_t size = bred + bsiz - pos;
        already_buffred = size;
        memcpy((void*)ptr, (uint8_t*)stream->buffer + (pos - bred), size);
        length -= size;
        ptr = (uint8_t*)ptr + size;
        SYS_FSEEK(file, stream->buffered + size);
    }

    // read the data that isn't buffered
    size_t advance = 0;
    while (length > 0)
    {
        stream->buffered = SYS_FTELL(file);
        size_t res = SYS_FREAD(file, stream->buffer, stream->buffer_size);

        if(res != stream->buffer_size && !file->eof)
            break;
        
        size_t readable = length < stream->buffer_size ? length : stream->buffer_size;
        readable = res < readable ? res : readable;
        memcpy((uint8_t*)ptr + advance, stream->buffer, readable);
        length -= readable;
        advance += readable;
        SYS_FSEEK(file, stream->buffered + readable);

        if(file->eof)
            break;
    }

    return advance + already_buffred;
}

static size_t fwrite_unbuffered(const void* ptr, size_t length, FILE* stream)
{
    return SYS_FWRITE(get_file(stream), (void*)ptr, length);
}

static size_t fwrite_linebuffered(const void* ptr, size_t length, FILE* stream)
{
    asm ("int 15");
    return -1;
}

static size_t fwrite_fullybuffered(const void* ptr, size_t length, FILE* stream)
{
    file_t* file = get_file(stream);

    // If nothing is buffered, buffer something
    if(stream->buffered == -1)
    {
        stream->buffered = SYS_FTELL(file);
        memset(stream->buffer, 0, stream->buffer_size);
    }

    size_t pos = SYS_FTELL(file);

    // writes the largest amount of bytes to the buffer
    size_t bytes_buffered = pos - stream->buffered;
    size_t free_buffer = (stream->buffer_size - bytes_buffered);
    size_t writable = length < free_buffer ? length : free_buffer;
    memcpy((uint8_t*)stream->buffer + bytes_buffered, ptr, writable);
    SYS_FSEEK(file, pos + writable);
    pos += writable;
    length -= writable;

    size_t writed = writable;

    // if there is still space in the buffer return
    if(pos - stream->buffered != stream->buffer_size)
        return writable;

    // flush the buffer and write the rest of the bytes
    while(length > 0)
    {
        // flush
        size_t res = SYS_FWRITE(file, stream->buffer, stream->buffer_size);
        
        if(res != stream->buffer_size || file->error)
            break;

        // update buffer
        stream->buffered = SYS_FTELL(file);
        size_t to_write = stream->buffer_size < length ? stream->buffer_size : length;
        memcpy(stream->buffer, (uint8_t*)ptr + writed, to_write);
        SYS_FSEEK(file, stream->buffered + to_write);
        length -= to_write;
        writed += to_write;
    }

    return writed;
}

int fflush(FILE* stream)
{
    if(!stream)
        return EOF;

    if(stream->buffered == -1)
        return 0;

    file_t* file = get_file(stream);
    if(file->mode == 'r')
        return 0;

    size_t pos = SYS_FTELL(file);
    SYS_FSEEK(file, stream->buffered);
    size_t bytes_buffered = pos - stream->buffered;
    size_t res = SYS_FWRITE(file, stream->buffer, bytes_buffered);
    SYS_FSEEK(file, pos);

    if(res != bytes_buffered)
        return EOF;

    return 0;
}

void setbuf(FILE* stream, char* buffer)
{
    if(!stream)
        return;

    if(buffer)
        setvbuf(stream, buffer, _IOFBF, BUFSIZ);
    else   
        setvbuf(stream, 0, _IONBF, 0);
}

int setvbuf(FILE* stream, char* buffer, int mode, size_t size)
{
    if(!stream)
        return -1;

    if(mode != _IOLBF && mode != _IOFBF && mode != _IONBF)
        return -1;

    if(buffer && size == 0)
        return - 1;

    fflush(stream);

    if(stream->flags & _IOLBF)
        stream->flags &= ~(_IOLBF);
    if(stream->flags & _IONBF)
        stream->flags &= ~(_IONBF);
    if(stream->flags & _IOFBF)
        stream->flags &= ~(_IOFBF);

    stream->flags |= mode;

    if(mode == _IONBF)
    {
        stream->buffer = 0;
        stream->buffer_size = 0;
    }
    else if(buffer)
    {
        stream->buffer = buffer;
        stream->buffer_size = size;
    }
    else
    {
        size_t index = (ptrdiff_t)&(opened_files) - (ptrdiff_t)stream;
        index /= sizeof(FILE);
        stream->buffer = (void*)&(buffers[index]);
        stream->buffer_size = BUFSIZ;
    }

    stream->buffered = -1;

    return 0;
}

int remove(const char* filename)
{
    return !SYS_REMOVE(filename);
}

FILE* fopen(const char* filename, const char* mode)
{
    int index = -1;
    for(int i = 0; i < FOPEN_MAX; i++)
        if(opened_files[i].flags == FS_INVALID)
        {
            index = i;
            break;
        }
    if(index == -1)
        return NULL;

    uint8_t flags = _IOFBF;
    switch (mode[0])
    {
    case 'r':
        flags |= MODE_READ | STATUS_READING;
        break;
    case 'w':
        flags |= MODE_WRITE | STATUS_WRITING;
        break;
    case 'a':
        flags |= MODE_APPEND | STATUS_WRITING;
        break;
    default:
        return NULL;
    }
    if(mode[1] == '+')
    {
        flags |= MODE_UPDATE;
        flags &= ~(STATUS_READING);
        flags &= ~(STATUS_WRITING);
    }

    SYS_FOPEN(filename, mode, &(opened_files[index]));
    if(opened_files[index].flags == FS_INVALID)
        return NULL;

    FILE* stream = &(object_files[index]);
    stream->flags = flags;
    stream->address = &(opened_files[index]);

    return stream;
}

int fclose(FILE* stream)
{
    if(!stream)
        return EOF;

    file_t* file = get_file(stream);
    if(!file)
        return EOF;
        
    fflush(stream);

    SYS_FCLOSE(file);
    return 0;
}

int fgetc(FILE* stream)
{
    if(!stream)
        return EOF;

    int c = 0;
    if(fread(&c, 1, 1, stream) != 1)
        return EOF;

    return c;
}

char* fgets(char* str, int num, FILE* stream)
{
    if(!stream || !str)
        return 0;

    char* ptr = str;
    do {
        if(fread(ptr, 1, 1, stream) != 1)
            return 0;
        if(*ptr++ == '\n')
            break;
    } while((ptr - str) < (num - 1));
    *ptr = '\0';

    return str;
}

int fputc(int character, FILE* stream)
{
    if(!stream)
        return EOF;

    if(fwrite(&character, 1, 1, stream) != 1)
        return EOF;
    
    return character;
}

int fputs(const char* str, FILE* stream)
{
    if(!stream || !str)
        return EOF;

    size_t len = strlen(str);

    if(fwrite(str, 1, len, stream) != len)
        return EOF;

    return 0;
}

int getchar()
{
    return fgetc(stdin);
}

char* gets(char* str)
{
    if(!str)
        return 0;

    char buff[512];
    char* res = fgets(buff, 512, stdin);

    if(!res)
        return 0;

    buff[strlen(buff) - 1] = '\0';
    memcpy(str, buff, strlen(buff) + 1);
    return str;
}

int putchar(int character)
{
    return fputc(character, stdout);
}

int puts(const char* str)
{
    static const char* nl = "\n";
    if(fputs(str, stdout) == EOF)
        return EOF;
    return fputs(nl, stdout);
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    if(!stream)
        return 0;

    if(size == 0 || count == 0)
        return 0;

    if(stream == (FILE*)STREAM_STDIN)
    {
        SYS_FREAD((file_t*)FSYS_STDIN, ptr, size * count);
        return count;
    }

    if(stream->flags & STATUS_WRITING)
        return 0;
    stream->flags |= STATUS_READING;

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    if(file->eof)
        return EOF;

    if(stream->flags & _IONBF)
        return fread_unbuffered(ptr, size * count, stream);
    else if(stream->flags & _IOLBF)
        return fread_linebuffered(ptr, size * count, stream);
    else if(stream->flags & _IOFBF)
        return fread_fullybuffered(ptr, size * count, stream);

    file->error = true;
    return 0;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
    if(!stream)
        return 0;

    if(size == 0 || count == 0)
        return 0;

    if(stream == (FILE*)STREAM_STDOUT || stream == (FILE*)STREAM_STDERR)
    {
        SYS_FWRITE((file_t*)FSYS_STDOUT, (void*)ptr, size * count);
        return count;
    }

    if(stream->flags & STATUS_READING)
        return 0;
    stream->flags |= STATUS_WRITING;

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    if(stream->flags & _IONBF >= _IONBF)
        return fwrite_unbuffered(ptr, size * count, stream) / size;
    else if(stream->flags & _IOLBF)
        return fwrite_linebuffered(ptr, size * count, stream) / size;
    else if(stream->flags & _IOFBF)
        return fwrite_fullybuffered(ptr, size * count, stream) / size;

    file->error = true;
    return 0;
}

int fgetpos(FILE* stream, fpos_t* pos)
{
    if(!stream)
        return -1;

    file_t* file = get_file(stream);
    if(!file)
        return -1;

    *pos = SYS_FTELL(file);

    return 0;
}

int fseek(FILE* stream, long int offset, int origin)
{
    if(!stream)
        return -1;

    file_t* file = get_file(stream);
    if(!file)
        return -1;

    fflush(stream);

    switch (origin)
    {
    case SEEK_SET:
        SYS_FSEEK(file, offset);
        break;
    case SEEK_CUR:
        SYS_FSEEK(file, SYS_FTELL(file) + offset);
        break;
    case SEEK_END:
        SYS_FSEEK(file, file->length + offset);
        break;    
    default:
        return -1;
    }

    if(stream->flags & MODE_UPDATE)
        if(stream->flags & STATUS_WRITING)
        {
            stream->flags &= ~(STATUS_WRITING);
            stream->flags |= STATUS_READING;
        }
        else
        {
            stream->flags &= ~(STATUS_READING);
            stream->flags |= STATUS_WRITING;
        }

    file->eof = false;
    return 0;
}

int fsetpos(FILE* stream, const fpos_t* pos)
{
    if(!stream)
        return -1;

    file_t* file = get_file(stream);
    if(!file)
        return -1;

    fflush(stream);

    SYS_FSEEK(file, *pos);

    if(stream->flags & MODE_UPDATE)
        if(stream->flags & STATUS_WRITING)
        {
            stream->flags &= ~(STATUS_WRITING);
            stream->flags |= STATUS_READING;
        }
        else
        {
            stream->flags &= ~(STATUS_READING);
            stream->flags |= STATUS_WRITING;
        }

    file->eof = false;
    return 0;
}

long int ftell(FILE* stream)
{
    if(!stream)
        return -1;

    file_t* file = get_file(stream);
    if(!file)
        return -1;

    return SYS_FTELL(file);
}

void rewind(FILE* stream)
{
    if(!stream)
        return;

    file_t* file = get_file(stream);
    if(!file)
        return;

    fflush(stream);

    SYS_FSEEK(file, 0);

    if(stream->flags & MODE_UPDATE)
        if(stream->flags & STATUS_WRITING)
        {
            stream->flags &= ~(STATUS_WRITING);
            stream->flags |= STATUS_READING;
        }
        else
        {
            stream->flags &= ~(STATUS_READING);
            stream->flags |= STATUS_WRITING;
        }

    file->eof = false;
    file->error = false;
}

int feof(FILE* stream)
{
    if(!stream)
        return 0;

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    return file->eof;
}

int ferror (FILE* stream)
{
    if(!stream)
        return 0;

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    return file->error;
}