#include <filesystem/pipefs.h>
#include <string.h>
#include <hash_table.h>
#include <queue.h>

typedef struct pipe
{
    char name[16];
    size_t refs;
    queue_t queue;
} pipe_t;

typedef struct data
{
    pipe_t* pipe;
} data_t;

static hash_table_t pipes;

file_system_t pipefs_generate(size_t disk_id, size_t offset, size_t size)
{
    (void)disk_id; (void)offset; (void)size;
    file_system_t fs;
    memset(&fs, 0, sizeof(fs));
    return fs;
}

bool pipefs_mount(fs_data_t* fs)
{
    pipes = hash_table_create(char[16], pipe_t);
    (void)fs;
    return false;
}

file_t pipefs_open_file(fs_data_t* fs, const char* filename, const char* mode)
{
    (void)fs;

    file_t file;
    memset(&file, 0, sizeof(file_t));
    strcpy(file.name, filename);
    file.mode = *mode;
    file.flags = FS_PIPE;

    pipe_t* pipe = __hash_table_at(pipes, file.name);
    if(!pipe)
    {
        pipefs_create_file(0, filename);
        pipe = __hash_table_at(pipes, file.name);
    }
    pipe->refs++;
    ((data_t*)file.data)->pipe = pipe;

    return file;
}

bool pipefs_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;

    pipe_t* pipe = ((data_t*)file->data)->pipe;
    pipe->refs--;
    if(pipe->refs == 0)
        pipefs_delete_file(fs, file->name);
    memset(file, 0, sizeof(file_t));
}

size_t pipefs_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs;
    
    if(!file || !buffer)
        return 0;

    uint8_t* buff = (uint8_t*)buffer;
    pipe_t* pipe = ((data_t*)file->data)->pipe;

    for(size_t i = 0; i < length; i++)
    {
        while(queue_empty(pipe->queue))
            asm("pause");
        
        *buff++ = *(uint8_t*)queue_front(pipe->queue);
        queue_pop(pipe->queue);
    }

    return length;
}

size_t pipefs_write_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    (void)fs;

    if(!file || !buffer)
        return 0;

    pipe_t* pipe = ((data_t*)file->data)->pipe;
    for(size_t i = 0; i < length; i++)
        queue_push(pipe->queue, ((uint8_t*)buffer)[i]);

    return length;
}

bool pipefs_create_file(fs_data_t* fs, const char* filename)
{
    (void)fs;

    if(!filename)
        return false;

    pipe_t pipe;
    strcpy(pipe.name, filename);
    pipe.refs = 1;
    pipe.queue = queue_create(uint8_t);
    __hash_table_insert(pipes, pipe.name, &pipe);

    return true;
}

bool pipefs_delete_file(fs_data_t* fs, const char* filename)
{
    (void)fs;

    if(!filename)
        return false;

    char name[16];
    strcpy(name, filename);
    pipe_t* pipe = __hash_table_at(pipes, name);
    if(!pipe)
        return false;

    queue_destroy(pipe->queue);
    __hash_table_erase(pipes, pipe->name);

    return true;
}

bool pipefs_create_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

bool pipefs_delete_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}

size_t pipefs_get_position(fs_data_t* fs, file_t* file)
{
    (void)fs; (void)file;
    return 0;
}

void pipefs_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    (void)fs; (void)file; (void)position;
}

bool pipefs_list_dir(fs_data_t* fs, size_t* index, char* filename, const char* dirpath)
{
    (void)fs; (void)index; (void)filename; (void)dirpath;
    return false;
}

bool pipefs_exists_file(fs_data_t* fs, const char* filename)
{
    (void)fs;

    char name[16];
    strcpy(name, filename);
    pipe_t* pipe = __hash_table_at(pipes, name);
    return pipe != 0;
}

bool pipefs_exists_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs; (void)dirpath;
    return false;
}