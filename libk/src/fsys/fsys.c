#include <fsys/fsys.h>

#include <string.h>
#include <stdlib.h>

#include <containers/darray.h>
#include <containers/trie.h>

typedef struct fsys_reg
{
    fsys_instantiate_t instantiate;
    uint8_t type;
} fsys_reg_t;

static fsys_reg_t* fsys_registry;
static trie_t fsys_instances;

static file_system_t* fsys_instantiate(partition_descriptor_t partition)
{
    file_system_t* fs = malloc(sizeof(file_system_t));
    memset(fs, 0, sizeof(file_system_t));
    
    for(size_t i = 0; i < darray_length(fsys_registry); i++)
    {
        if(fsys_registry[i].type == partition.type)
        {
            *fs = fsys_registry[i].instantiate(partition);
            break;
        }
    }

    fs->type = partition.type;
    return fs;
}

void fsys_init()
{
    fsys_registry = darray(fsys_reg_t, 0);
    fsys_instances = trie();
}

void fsys_auto_mount()
{
    char id[] = { 'a', '\0' };
    for(size_t dev = 0; dev < storage_ndevices(); dev++)
    {
        storage_descriptor_t desc = storage_info(dev);
        
        for (size_t part = 0; part < desc.partitions_count; part++)
        {
            partition_descriptor_t partition = storage_get_partition(dev, part);
            fsys_mount(partition, id);
            id[0]++;
        }
    }
}

void fsys_register(fsys_instantiate_t instantiate_function, uint8_t type)
{
    fsys_reg_t reg;
    reg.instantiate = instantiate_function;
    reg.type = type;
    darray_append(fsys_registry, reg);
}

bool fsys_mount(partition_descriptor_t partition, const char* name)
{
    file_system_t* fs = fsys_instantiate(partition);
    if(fs->type == 0)
        return false;
    
    fs->data.disk_id = partition.device_id;
    fs->data.offset = partition.start;
    fs->data.length = partition.length;

    return trie_insert(fsys_instances, name, fs);
}

bool fsys_mount_vfs(fsys_instantiate_t instantiate_function, const char* name)
{
    file_system_t* fs = malloc(sizeof(file_system_t));
    *fs = instantiate_function((partition_descriptor_t){});
    return trie_insert(fsys_instances, name, fs);
}

void fsys_unmount(const char* name)
{
    free(trie_get(fsys_instances, name));
    trie_remove(fsys_instances, name);
}

uint8_t fsys_type(const char* name)
{
    file_system_t* fs = trie_get(fsys_instances, name);
    if(fs == NULL)
        return 0;
    return fs->type;
}

static const char* split_name(const char* filename, char** fname)
{
    char* separator = (char*)strchr(filename, ':');
    *fname = separator + 1;
    *separator = '\0';
    return filename;
}

file_t fsys_open_file(const char* filename, fsys_file_mode_t mode)
{
    if(!filename)
        return INVALID_FILE;

    char* fname;
    const char* fsys = split_name(filename, &fname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return INVALID_FILE;
    
    file_t file = fs->open_file(&fs->data, fname, mode);
    file.fsys = fs;
    return file;
}

bool fsys_close_file(file_t* file)
{
    if(!file)
        return false;

    file_system_t* fs = file->fsys;
    return fs->close_file(&fs->data, file);
}

size_t fsys_read_file(file_t* file, void* buffer, size_t length)
{
    if(!file || !buffer || length == 0)
        return 0;

    file_system_t* fs = file->fsys;
    return fs->read_file(&fs->data, file, buffer, length);
}

size_t fsys_write_file(file_t* file, void* buffer, size_t length)
{
    if(!file || !buffer || length == 0)
        return 0;

    file_system_t* fs = file->fsys;
    return fs->write_file(&fs->data, file, buffer, length);
}

bool fsys_create_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    const char* fsys = split_name(filename, &fname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->create_file(&fs->data, fname);
}

bool fsys_delete_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    const char* fsys = split_name(filename, &fname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->delete_file(&fs->data, fname);
}

bool fsys_create_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dname;
    const char* fsys = split_name(dirpath, &dname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->create_dir(&fs->data, dname);
}

bool fsys_delete_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dname;
    const char* fsys = split_name(dirpath, &dname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->delete_dir(&fs->data, dname);
}

size_t fsys_get_position(file_t* file)
{
    if(!file)
        return 0;

    file_system_t* fs = file->fsys;
    return fs->get_position(&fs->data, file);
}

bool fsys_set_position(file_t* file, size_t offset)
{
    if(!file)
        return false;

    file_system_t* fs = file->fsys;
    return fs->set_position(&fs->data, file, offset);
}

bool fsys_exists_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    const char* fsys = split_name(filename, &fname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->exists_file(&fs->data, fname);
}

bool fsys_exists_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* dname;
    const char* fsys = split_name(dirpath, &dname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return false;
    
    return fs->exists_dir(&fs->data, dname);
}

file_t fsys_open_dir(const char* dirpath)
{
    if(!dirpath)
        return INVALID_FILE;

    char* dname;
    const char* fsys = split_name(dirpath, &dname);
    file_system_t* fs = trie_get(fsys_instances, fsys);
    if(!fs)
        return INVALID_FILE;
    
    file_t dir = fs->open_dir(&fs->data, dname);
    dir.fsys = fs;
    return dir;
}

bool fsys_list_dir(file_t* dir, direntry_t* entry)
{
    if(!dir || !entry)
        return false;

    file_system_t* fs = dir->fsys;
    return fs->list_dir(&fs->data, dir, entry);
}

bool fsys_close_dir(file_t* dir)
{
    if(!dir)
        return false;

    file_system_t* fs = dir->fsys;
    return fs->close_dir(&fs->data, dir);
}
