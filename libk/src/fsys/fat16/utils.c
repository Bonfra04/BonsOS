#include <storage/storage.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "fat16_utils.h"

fat16_entry_t* unpack_file(const file_t* file)
{
    return (fat16_entry_t*)file->fs_data;
}

file_t pack_file(fat16_entry_t entry)
{
    file_t file;
    memset(&file, 0, sizeof(file_t));
    *(fat16_entry_t*)(&file.fs_data) = entry;
    return file;
}

bool get_next_cluster(const fat16_data_t* data, uint64_t current_cluster, uint64_t* next_cluster)
{
    uint64_t addr = data->fat_offset + current_cluster * sizeof(uint16_t);

    *next_cluster = 0;
    if(storage_seek_read(data->storage_id, data->offset + addr, sizeof(uint16_t), next_cluster) != sizeof(uint16_t))
        return false;

    return !(*next_cluster >= 0xFFF8 && *next_cluster <= 0xFFFF);
}

void from_dos(char dos[8+3], char* name, uint8_t upplow_mask)
{
    uint64_t advance = 0;

    bool name_case = (upplow_mask >> 3) & 1;
    bool ext_case = (upplow_mask >> 4) & 1;

    for(int i = 0; i < 8; i++)
        if(dos[i] != ' ')
            name[advance++] = name_case ? tolower(dos[i]) : toupper(dos[i]);
        else
            break;

    if(dos[8] != ' ')
        name[advance++] = '.';

    for(int i = 8; i < 11; i++)
        if(dos[i] != ' ')
            name[advance++] = ext_case ? tolower(dos[i]) : toupper(dos[i]);
        else
            break;

    name[advance] = '\0';
}

size_t get_pos(const fat16_entry_t* entry)
{
    return entry->advance;
}

bool set_pos(const fat16_data_t* data, fat16_entry_t* entry, size_t position)
{
    if(entry->type == FAT16_FILE && position > entry->length)
        position = entry->length;

    uint64_t nclusters = position / data->bytes_per_cluster;
    uint64_t offset = position % data->bytes_per_cluster;

    entry->cluster = entry->first_cluster;

    while(nclusters--)
        if(!get_next_cluster(data, entry->cluster, &entry->cluster))
            return false;

    entry->cluster_offset = offset;
    entry->advance = position;

    return true;
}

void direntry_to_fatentry(const direntry_t* d, fat16_entry_t* entry, uint64_t entry_addr)
{
    fat16_direntry_t dir = *(fat16_direntry_t*)(&d->fs_data);
    entry->first_cluster = dir.dir_entry.first_cluster;
    entry->type = dir.dir_entry.flags & ENTRY_DIRECTORY ? FAT16_DIR : FAT16_FILE;
    entry->length = dir.dir_entry.file_size;
    entry->cluster = dir.dir_entry.first_cluster;
    entry->cluster_offset = 0;
    entry->advance = 0;
    entry->lfn = dir.lfn;
    entry->entry_addr = entry_addr;
}

bool allocate_cluster(const fat16_data_t* data, uint64_t current_cluster, uint64_t* new_cluster)
{
    if(!storage_seek(data->storage_id, data->offset + data->fat_offset))
        return false;

    uint16_t cluster = 0;

    for(; cluster < data->data_cluster_count - FIRST_CLUSTER_OFFSET; cluster++)
    {
        uint16_t entry;
        if(storage_read(data->storage_id, sizeof(uint16_t), &entry) != sizeof(uint16_t))
            return false;

        if(entry == 0)
        {
            entry = 0xFFFF;
            if(storage_seek_write(data->storage_id, data->offset + data->fat_offset + cluster * sizeof(uint16_t), sizeof(uint16_t), &entry) != sizeof(uint16_t))
                return false;
            break;
        }
    }

    if(cluster == data->data_cluster_count - FIRST_CLUSTER_OFFSET)
        return false;

    if(current_cluster != 0)
        if(storage_seek_write(data->storage_id, data->offset + data->fat_offset + current_cluster * sizeof(uint16_t), sizeof(uint16_t), &cluster) != sizeof(uint16_t))
            return false;

    *new_cluster = cluster;
    return true;
}

uint64_t dir_chain_len(const fat16_data_t* data, const fat16_entry_t* dir)
{
    fat16_entry_t directory = *dir;
    uint64_t len = 0;

    direntry_t dirent;
    while(list_dir(data, &directory, &dirent))
    {
        bool* del = &((fat16_direntry_t*)dirent.fs_data)->deleted;
        if(!*del)
            return len;
        len++;
    }

    return UINT64_MAX;
}

static bool is_valid_char(char c)
{
    return
        c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c >= 127 ||
        c == ' ' || c == '$' || c == '%' || c == '-' || c == '_' || c == '@' || c == '~' ||
        c == '`' || c == '!' || c == '(' || c == ')' || c == '{' || c == '}' || c == '^' ||
        c == '#' || c == '&';
}

static bool to_dos(const char* name, char dos[8+3], uint8_t* caseness)
{
    if(strlen(name) > 12)
        return false;

    memset(dos, ' ', 8+3);

    uint8_t t;
    for(t = 0; name[t] && !isalnum(name[t]); t++);
    bool name_case = islower(name[t]);

    uint8_t i;
    for(i = 0; i < 8 && name[i] && name[i] != '.'; i++)
    {
        if(!is_valid_char(name[i]))
            return false;

        if((name_case && isupper(name[i])) || (!name_case && islower(name[i])))
            return false;

        dos[i] = toupper(name[i]);
    }

    if(name[i] == '\0')
    {
        *caseness = name_case << 3;
        return true;
    }

    if(name[i] != '.')
        return false;

    i++;

    for(t = i; name[t] && !isalnum(name[t]); t++);
    bool ext_case = islower(name[t]);

    for(uint8_t j = 8; j < 11 && name[i]; j++, i++)
    {
        if(!is_valid_char(name[i]))
            return false;

        if((ext_case && isupper(name[i])) || (!ext_case && islower(name[i])))
            return false;

        dos[j] = toupper(name[i]);
    }

    *caseness = (name_case << 3) | (ext_case << 4);
    return name[i] == '\0';
}

static bool check_available_name(const fat16_data_t* data, const fat16_entry_t* dir, const char dos[8+3])
{
    fat16_entry_t directory = *dir;
    dir_entry_t dirent;
    while(memcmp(dirent.fullname, dos, 8+3) != 0)
    {
        if(read_entry(data, &directory, &dirent, sizeof(dir_entry_t)) != sizeof(dir_entry_t))
            return false;

        if(dirent.fullname[0] == '\0')
            return true;
    }
    return false;
}

static void shortenize_name(const fat16_data_t* data, const fat16_entry_t* dir, const char* longname, char dos[8+3])
{
    if(strlen(longname) == 12)
    {
        memcpy(dos, longname, 8);
        memcpy(dos + 8, longname + 8 + 1, 3);
        strtoupper(dos);

        if(check_available_name(data, dir, dos))
            return;
    }

    uint32_t n = 0;
    char numbuf[9];

    do
    {
        uitoa(n, numbuf, 16);
        size_t ndigits = strlen(numbuf);

        size_t tilde_idx = 11 - ndigits - 1;
        memcpy(dos, longname, tilde_idx);
        dos[tilde_idx] = '~';
        memcpy(dos + tilde_idx + 1, numbuf, ndigits);

        n++;
    } while(!check_available_name(data, dir, dos));
}

static uint8_t lfn_checksum(const uint8_t* dos)
{
    int i;
    uint8_t sum = 0;

    for (i = 11; i; i--)
        sum = ((sum & 1) << 7) + (sum >> 1) + *dos++;

    return sum;
}

dir_entry_t* gen_entries(const fat16_data_t* data, const fat16_entry_t* dir, const char* entryname, size_t* num_entries, uint8_t flags)
{
    dir_entry_t* entries = malloc(sizeof(dir_entry_t));
    entries[0] = construct_dir_entry(flags);

    uint8_t caseness;
    if(to_dos(entryname, entries->fullname, &caseness))
    {
        *num_entries = 1;
        entries->upplow_mask = caseness;
        return entries;
    }

    size_t name_len = strlen(entryname);
    *num_entries = name_len / CHARS_PER_LFN + (name_len % CHARS_PER_LFN != 0) + 1;
    entries = realloc(entries, sizeof(dir_entry_t) * *num_entries);

    dir_entry_t* short_83 = &entries[*num_entries - 1];
    memmove(entries, short_83, sizeof(dir_entry_t));

    shortenize_name(data, dir, entryname, short_83->fullname);
    uint8_t chsum = lfn_checksum((uint8_t*)short_83->fullname);


    for(size_t i = *num_entries; i > 1; i--)
    {
        uint64_t adv = 0;

        uint8_t order = i - 1;
        lfn_entry_t* lfn = (lfn_entry_t*)&entries[*num_entries - i];
        memset(lfn, 0, sizeof(lfn_entry_t));
        lfn->lfn_attr = ENTRY_LFN;
        lfn->order = order | (i == *num_entries ? 0x40 : 0);
        lfn->checksum = chsum;

        for(uint8_t i = 0; i < 5; i++)
        {
            size_t idx = CHARS_PER_LFN * (order - 1) + adv++;
            lfn->name0[i] = idx < name_len ? entryname[idx] : 0xFFFF;
        }
        for(uint8_t i = 0; i < 6; i++)
        {
            size_t idx = CHARS_PER_LFN * (order - 1) + adv++;
            lfn->name1[i] = idx < name_len ? entryname[idx] : 0xFFFF;
        }
        for(uint8_t i = 0; i < 2; i++)
        {
            size_t idx = CHARS_PER_LFN * (order - 1) + adv++;
            lfn->name2[i] = idx < name_len ? entryname[idx] : 0xFFFF;
        }
    }

    return entries;
}

dir_entry_t construct_dir_entry(uint8_t flags)
{
    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));
    entry.flags = flags;
    // TODO: times
    return entry;
}

bool free_cluster_chain(const fat16_data_t* data, uint64_t first_cluster)
{
    uint64_t cluster = first_cluster;
    while(cluster != 0 && !(cluster >= 0xFFF8 && cluster <= 0xFFFF))
    {
        uint64_t next_cluster;
        if(!get_next_cluster(data, cluster, &next_cluster) && next_cluster != 0xFFFF)
            return false;

        uint16_t free_cluster = 0;
        uint64_t addr = data->fat_offset + cluster * sizeof(uint16_t);
        if(storage_seek_write(data->storage_id, data->offset + addr, sizeof(uint16_t), &free_cluster) != sizeof(uint16_t))
            return false;

        cluster = next_cluster;
    }

    return true;
}
