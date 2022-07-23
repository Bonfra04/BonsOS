#include <fsys/fsys.h>

#include <string.h>
#include <stdlib.h>

#include <containers/darray.h>

typedef struct fsys_reg
{
    fsys_instantiate_t instantiate;
    uint8_t type;
} fsys_reg_t;

static fsys_reg_t* fsys_registry;
static file_system_t* fsys_instances;

static file_system_t fsys_instantiate(partition_descriptor_t partition)
{
    file_system_t fs;
    memset(&fs, 0, sizeof(file_system_t));
    
    for(size_t i = 0; i < darray_length(fsys_registry); i++)
    {
        if(fsys_registry[i].type == partition.type)
        {
            fs = fsys_registry[i].instantiate(partition);
            break;
        }
    }

    fs.mounted = false;
    fs.type = partition.type;
    return fs;
}

void fsys_init()
{
    fsys_registry = darray(fsys_reg_t, 0);
    fsys_instances = darray(file_system_t, 0);
}

void fsys_auto_mount()
{
    for(size_t dev = 0; dev < storage_ndevices(); dev++)
    {
        storage_descriptor_t desc = storage_info(dev);
        
        for (size_t part = 0; part < desc.partitions_count; part++)
        {
            partition_descriptor_t partition = storage_get_partition(dev, part);
            fsys_mount(partition);
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

uint64_t fsys_mount(partition_descriptor_t partition)
{
    file_system_t fs = fsys_instantiate(partition);
    if(fs.type == 0)
        return 0;
    
    fs.mounted = true;
    fs.data.disk_id = partition.device_id;
    fs.data.offset = partition.start;
    fs.data.length = partition.length;

    for(size_t i = 0; i < darray_length(fsys_instances); i++)
    {
        if(!fsys_instances[i].mounted)
        {
            fsys_instances[i] = fs;
            return i + 1;
        }
    }

    darray_append(fsys_instances, fs);
    return darray_length(fsys_instances);
}

void fsys_unmount(uint64_t id)
{
    if(id == 0 || id > darray_length(fsys_instances))
        return;

    memset(&fsys_instances[id - 1], 0, sizeof(file_system_t));
    fsys_instances[id - 1].mounted = false;
}

uint64_t fsys_nfsys()
{
    uint64_t count = 0;
    for(size_t i = 0; i < darray_length(fsys_instances); i++)
    {
        if(fsys_instances[i].mounted)
            count++;
    }
    return count;
}

uint8_t fsys_type(uint64_t id)
{
    if(id == 0 || id > darray_length(fsys_instances))
        return 0;

    return fsys_instances[id - 1].type;
}

static uint64_t split_name(const char* filename, char** fname)
{
    const char* separator = strchr(filename, ':');
    *fname = (char*)separator + 1;

    char fsys[separator - filename + 1];
    memcpy(fsys, filename, separator - filename);
    fsys[separator - filename] = '\0';

    return atoull(fsys);
}

file_system_t* get_fs(uint64_t id)
{
    if(id == 0 || id > darray_length(fsys_instances))
        return NULL;
    
    if(!fsys_instances[id - 1].mounted)
        return NULL;

    return &fsys_instances[id - 1];
}

file_t fsys_open_file(const char* filename, fsys_file_mode_t mode)
{
    if(!filename)
        return INVALID_FILE;

    char* fname;
    uint64_t fsys = split_name(filename, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return INVALID_FILE;
    
    file_t file = fs->open_file(&fs->data, fname, mode);
    file.fsys_id = fsys;
    return file;
}

bool fsys_close_file(file_t* file)
{
    if(!file)
        return false;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return false;

    return fs->close_file(&fs->data, file);
}

size_t fsys_read_file(file_t* file, void* buffer, size_t length)
{
    if(!file || !buffer || length == 0)
        return 0;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return 0;

    return fs->read_file(&fs->data, file, buffer, length);
}

size_t fsys_write_file(file_t* file, void* buffer, size_t length)
{
    if(!file || !buffer || length == 0)
        return 0;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return 0;

    return fs->write_file(&fs->data, file, buffer, length);
}

bool fsys_create_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    uint64_t fsys = split_name(filename, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->create_file(&fs->data, fname);
}

bool fsys_delete_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    uint64_t fsys = split_name(filename, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->delete_file(&fs->data, fname);
}

bool fsys_create_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* fname;
    uint64_t fsys = split_name(dirpath, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->create_dir(&fs->data, fname);
}

bool fsys_delete_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* fname;
    uint64_t fsys = split_name(dirpath, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->delete_dir(&fs->data, fname);
}

size_t fsys_get_position(file_t* file)
{
    if(!file)
        return 0;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return 0;

    return fs->get_position(&fs->data, file);
}

bool fsys_set_position(file_t* file, size_t offset)
{
    if(!file)
        return false;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return false;

    return fs->set_position(&fs->data, file, offset);
}

bool fsys_exists_file(const char* filename)
{
    if(!filename)
        return false;

    char* fname;
    uint64_t fsys = split_name(filename, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->exists_file(&fs->data, fname);
}

bool fsys_exists_dir(const char* dirpath)
{
    if(!dirpath)
        return false;

    char* fname;
    uint64_t fsys = split_name(dirpath, &fname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return false;
    
    return fs->exists_dir(&fs->data, fname);
}

file_t fsys_open_dir(const char* dirpath)
{
    if(!dirpath)
        return INVALID_FILE;

    char* dname;
    uint64_t fsys = split_name(dirpath, &dname);
    
    file_system_t* fs = get_fs(fsys);
    if(!fs)
        return INVALID_FILE;
    
    file_t dir = fs->open_dir(&fs->data, dname);
    dir.fsys_id = fsys;
    return dir;
}

bool fsys_list_dir(file_t* dir, direntry_t* entry)
{
    if(!dir || !entry)
        return false;

    file_system_t* fs = get_fs(dir->fsys_id);
    if(!fs)
        return false;

    return fs->list_dir(&fs->data, dir, entry);
}

bool fsys_close_dir(file_t* dir)
{
    if(!dir)
        return false;

    file_system_t* fs = get_fs(dir->fsys_id);
    if(!fs)
        return false;

    return fs->close_dir(&fs->data, dir);
}

bool fsys_error(file_t* file)
{
    if(!file)
        return false;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return false;

    return fs->error(&fs->data, file);
}

bool fsys_eof(file_t* file)
{
    if(!file)
        return false;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return false;

    return fs->eof(&fs->data, file);
}

void fsys_clear_error(file_t* file)
{
    if(!file)
        return;

    file_system_t* fs = get_fs(file->fsys_id);
    if(!fs)
        return;

    fs->clear_error(&fs->data, file);
}
