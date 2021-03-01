#include <stdio.h>
#include <filesystem/fsys.h>

static file_t opened_files[FOPEN_MAX];
static FILE object_files[FOPEN_MAX];

void stdio_stream_init()
{
    for(int i = 0; i < FOPEN_MAX; i++)
        opened_files[i].flags = FS_INVALID;
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

    opened_files[index] = fsys_open_file(filename);
    if(opened_files[index].flags == FS_INVALID)
        return NULL;
    object_files[index].address = &(opened_files[index]);
    return &object_files[index];
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

    file_t* file = get_file(stream);
    if(!file)
        return 0;

    size_t res = fsys_read_file(file, ptr, size * count);
    if(res == 0)
        return 0;
    if(res == FS_EOF)
        return EOF;
    
    return res;
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
        if(!fsys_set_position(file, offset))
            return -1;
        break;
    case SEEK_CUR:
        if(!fsys_set_position(file, fsys_get_position(file) + offset))
            return -1;
        break;
    case SEEK_END:
        if(!fsys_set_position(file, file->length + offset))
            return -1;
        break;    
    default:
        return -1;
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