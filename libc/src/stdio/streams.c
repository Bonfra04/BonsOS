#include <stdio.h>
#include <string.h>
#include <filesystem/fsys.h>

static file_t opened_files[FOPEN_MAX];
static FILE object_files[FOPEN_MAX];

static char buffers[FOPEN_MAX][BUFSIZ];

#define MODE_READ       0b000001
#define MODE_WRITE      0b000010
#define MODE_APPEND     0b000100
#define MODE_UPDATE     0b001000
#define STATUS_READING  0b010000
#define STATUS_WRITING  0b100000

void stdio_stream_init()
{
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

static size_t fread_unbuffered(void* ptr, size_t length, FILE* stream)
{
    size_t res = fsys_read_file(get_file(stream), ptr, length);
    if(res == FS_EOF)
        return EOF;
    return res;
}

static size_t fread_linebuffered(void* ptr, size_t length, FILE* stream)
{
    get_file(stream)->error = true;
    asm("int 15");
    return 0;
}

static size_t fread_fullybuffered(void* ptr, size_t length, FILE* stream)
{
    file_t* file = get_file(stream);

    if(stream->buffered == -1)
    {
        stream->buffered = fsys_get_position(file);
        size_t res = fsys_read_file(file, stream->buffer, stream->buffer_size);
        if(res != stream->buffer_size && !file->eof)
            return 0;
        fsys_set_position(file, 0);
    }

    size_t pos = fsys_get_position(file);
    size_t bred = stream->buffered;
    size_t bsiz = stream->buffer_size;

    if(pos >= bred && (pos + length) <= (bred + bsiz))
    {
        memcpy(ptr, (uint8_t*)stream->buffer + (pos - bred), length);
        fsys_set_position(file, pos + length);
        return length;
    }
    else if(pos < bred && bred <= (pos + length) && (bred + bsiz) >= (pos + length))
    {
        size_t offset = length - (bred - pos);
        memcpy((uint8_t*)ptr + offset, stream->buffer, pos + length - bred);
        length -= pos + length - bred;
    }
    else if(bred <= pos && pos <= (bred + bsiz) && (bred + bsiz) <= (pos + length))
    {
        size_t size = bred + bsiz - pos;
        memcpy(ptr, (uint8_t*)stream->buffer + (pos - bred), size);
        length -= size;
        ptr = (uint8_t*)ptr + size;
        fsys_set_position(file, stream->buffered + size);
    }

    size_t advance = 0;
    while (length > 0)
    {
        stream->buffered = fsys_get_position(file);
        size_t res = fsys_read_file(file, stream->buffer, stream->buffer_size);

        if(res != stream->buffer_size && !file->eof)
            break;
        
        size_t readable = length < stream->buffer_size ? length : stream->buffer_size;
        readable = res < readable ? res : readable;
        memcpy((uint8_t*)ptr + advance, stream->buffer, readable);
        length -= readable;
        advance += readable;
        fsys_set_position(file, stream->buffered + readable);

        if(file->eof)
            break;
    }
    return advance;
}

int remove(const char* filename)
{
    return fsys_remove(filename);
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

    opened_files[index] = fsys_open_file(filename);
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
        
    fsys_close_file(file);
    return 0;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    if(!stream)
        return 0;

    if(size == 0 || count == 0)
        return 0;

    if(stream->flags & STATUS_WRITING)
        return 0;
    stream->flags |= STATUS_READING;

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    if(file->eof)
        return EOF;

    if((stream->flags & _IONBF) >= _IONBF)
        return fread_unbuffered(ptr, size * count, stream);
    else if(stream->flags & _IOLBF)
        return fread_linebuffered(ptr, size * count, stream);
    else if(stream->flags & _IOFBF)
        return fread_fullybuffered(ptr, size * count, stream);

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

    pos->current_cluster = file->cluster;
    pos->current_sector = file->sector;
    pos->current_position = file->position;

    return 0;
}

int fseek(FILE* stream, long int offset, int origin)
{
    if(!stream)
        return -1;

    file_t* file = get_file(stream);
    if(!file)
        return -1;

    switch (origin)
    {
    case SEEK_SET:
        fsys_set_position(file, offset);
        break;
    case SEEK_CUR:
        fsys_set_position(file, fsys_get_position(file) + offset);
        break;
    case SEEK_END:
        fsys_set_position(file, file->length + offset);
        break;    
    default:
        return -1;
    }

    if(file->flags & MODE_UPDATE)
        if(file->flags & STATUS_WRITING)
        {
            file->flags &= ~(STATUS_WRITING);
            file->flags |= STATUS_READING;
        }
        else
        {
            file->flags &= ~(STATUS_READING);
            file->flags |= STATUS_WRITING;
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

    file->cluster = pos->current_cluster;
    file->sector = pos->current_sector;
    file->position = pos->current_position;

    if(file->flags & MODE_UPDATE)
        if(file->flags & STATUS_WRITING)
        {
            file->flags &= ~(STATUS_WRITING);
            file->flags |= STATUS_READING;
        }
        else
        {
            file->flags &= ~(STATUS_READING);
            file->flags |= STATUS_WRITING;
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

    return fsys_get_position(file);
}

void rewind(FILE* stream)
{
    if(!stream)
        return;

    file_t* file = get_file(stream);
    if(!file)
        return;

    file->position = 0;
    file->sector = 0;
    file->cluster = 0;

    if(file->flags & MODE_UPDATE)
        if(file->flags & STATUS_WRITING)
        {
            file->flags &= ~(STATUS_WRITING);
            file->flags |= STATUS_READING;
        }
        else
        {
            file->flags &= ~(STATUS_READING);
            file->flags |= STATUS_WRITING;
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