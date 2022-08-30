#include <fsys/vfs/vfs_fb.h>
#include <graphics/screen.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct fb_entry
{
    uint64_t x;
    uint64_t y;
    uint64_t id;
    bool is_dir;
    uint64_t curr_id;
} fb_entry_t;

static fb_entry_t* unpack_file(const file_t* file)
{
    return (fb_entry_t*)file->fs_data;
}

static file_t pack_file(fb_entry_t entry)
{
    file_t file;
    memset(&file, 0, sizeof(file_t));
    *(fb_entry_t*)(&file.fs_data) = entry;
    return file;
}

static uint64_t get_id(const char* filename)
{
    size_t len = strlen(filename);
    for(size_t i = 0; i < len; i++)
        if(!isdigit(filename[i]))
            return -1;

    return atoull(filename + 1);
}

void vfs_fb_init()
{
    fsys_mount_vfs(vfs_fb_instantiate, "fb");
}

file_system_t vfs_fb_instantiate(partition_descriptor_t partition)
{
    (void)partition;

    file_system_t fs;
    fs.open_file = vfs_fb_open_file;
    fs.close_file = vfs_fb_close_file;
    fs.read_file = vfs_fb_read_file;
    fs.write_file = vfs_fb_write_file;
    fs.create_file = vfs_fb_create_file;
    fs.delete_file = vfs_fb_delete_file;
    fs.create_dir = vfs_fb_create_dir;
    fs.delete_dir = vfs_fb_delete_dir;
    fs.get_position = vfs_fb_get_position;
    fs.set_position = vfs_fb_set_position;
    fs.exists_file = vfs_fb_exists_file;
    fs.exists_dir = vfs_fb_exists_dir;
    fs.open_dir = vfs_fb_open_dir;
    fs.list_dir = vfs_fb_list_dir;
    fs.close_dir = vfs_fb_close_dir;
    fs.error = vfs_fb_error;

    return fs;
}

static bool is_valid_id(uint64_t id)
{
    // TODO: check for multiple framebuffers
    return id == 0;
}

file_t vfs_fb_open_file(fs_data_t* fs, const char* filename, fsys_file_mode_t mode)
{
    if(!vfs_fb_exists_file(fs, filename))
        return INVALID_FILE;

    fb_entry_t fb;
    fb.x = 0;
    fb.y = 0;
    fb.id = get_id(filename);
    fb.is_dir = false;

    return pack_file(fb);
}

bool vfs_fb_close_file(fs_data_t* fs, file_t* file)
{
    (void)fs;
    if(unpack_file(file)->is_dir)
        return false;
    *file = INVALID_FILE;
    return true;
}

size_t vfs_fb_read_file(fs_data_t* fs, file_t* file, void* buffer, size_t length)
{
    fb_entry_t* entry = unpack_file(file);
    if(entry->is_dir)
        return 0;

    size_t read = screen_get_area(entry->x, entry->y, length, buffer);

    entry->x += read % screen_get_width();
    entry->y += read / screen_get_width();

    return read;
}

size_t vfs_fb_write_file(fs_data_t* fs, file_t* file, const void* buffer, size_t length)
{
    fb_entry_t* entry = unpack_file(file);
    if(entry->is_dir)
        return 0;

    size_t drawn = screen_set_area(entry->x, entry->y, length, buffer);

    entry->x += drawn % screen_get_width();
    entry->y += drawn / screen_get_width();

    return drawn;
}

bool vfs_fb_create_file(fs_data_t* fs, const char* filename)
{
    (void)fs, (void)filename;
    return false;
}

bool vfs_fb_delete_file(fs_data_t* fs, const char* filename)
{
    (void)fs, (void)filename;
    return false;
}

bool vfs_fb_create_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs, (void)dirpath;
    return false;
}

bool vfs_fb_delete_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs, (void)dirpath;
    return false;
}

size_t vfs_fb_get_position(fs_data_t* fs, const file_t* file)
{
    (void)fs;
    fb_entry_t* entry = unpack_file(file);
    if(entry->is_dir)
        return 0;

    return entry->y * screen_get_width() + entry->x;
}

bool vfs_fb_set_position(fs_data_t* fs, file_t* file, size_t position)
{
    (void)fs;

    fb_entry_t* entry = unpack_file(file);
    if(entry->is_dir)
        return false;

    uint64_t x = position % screen_get_width();
    uint64_t y = position / screen_get_width();
    if(y >= screen_get_height())
        return false;

    entry->x = x;
    entry-> y = y;

    return true;
}

bool vfs_fb_exists_file(fs_data_t* fs, const char* filename)
{
    if(filename[0] != '/')
        return false;

    return is_valid_id(get_id(filename));
}

bool vfs_fb_exists_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs;
    return dirpath[0] == '/' && dirpath[1] == '\0';
}

file_t vfs_fb_open_dir(fs_data_t* fs, const char* dirpath)
{
    (void)fs;
    if(!vfs_fb_exists_dir(fs, dirpath))
        return INVALID_FILE;

    fb_entry_t entry;
    entry.is_dir = true;
    entry.curr_id = 0;
    return pack_file(entry);
}

bool vfs_fb_list_dir(fs_data_t* fs, file_t* dir, direntry_t* entry)
{
    (void)fs;
    fb_entry_t* fb = unpack_file(dir);
    if(!fb->is_dir)
        return false;

    if(!is_valid_id(fb->curr_id))
        return false;

    entry->flags = FSYS_FLG_FILE | FSYS_FLG_READ_ONLY;
    ulltoa(fb->curr_id, entry->name, 10);

    fb->curr_id++;
    return true;
}

bool vfs_fb_close_dir(fs_data_t* fs, file_t* dir)
{
    (void)fs;
    if(!unpack_file(dir)->is_dir)
        return false;
    *dir = INVALID_FILE;
    return true;
}

bool vfs_fb_error(fs_data_t* fs, const file_t* file)
{
    return false;
}
