#include <hash_table.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SIZE 8

#define get_ptable(hash_table) ((hash_table_data_t*)hash_table)

typedef struct entry
{
    void* key;
    void* value;
    struct entry* next_entry;
} entry_t;

typedef struct hash_table_data
{
    size_t capacity;
    size_t length;
    size_t key_size;
    size_t value_size;
    entry_t** entries;
} hash_table_data_t;

static uint64_t hash(uint8_t* key, size_t len)
{
    uint64_t hash = 2166136261;

    for (size_t i = 0; i < len; i++)
        hash = (hash ^ key[i]) * 16777619;

    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
}

static void free_entry(entry_t* entry)
{
    free(entry->key);
    free(entry->value);
    if(entry->next_entry)
        free_entry(entry->next_entry);
    free(entry);
}

static entry_t* make_pair(hash_table_data_t* ptable, void* key_ptr, void* value_ptr)
{
    entry_t* entry = (entry_t*)malloc(sizeof(entry_t));
    entry->next_entry = 0;
    entry->key = malloc(ptable->key_size);
    entry->value = malloc(ptable->value_size);

    memcpy(entry->key, key_ptr, ptable->key_size);
    memcpy(entry->value, value_ptr, ptable->value_size);

    return entry;
}

static void rehash(hash_table_data_t* ptable, size_t new_capacity)
{
    entry_t** entries = malloc(sizeof(entry_t*) * new_capacity);
    entry_t** old_entries = ptable->entries;
    ptable->entries = entries;

    uint64_t old_capacity = ptable->capacity;
    ptable->capacity = new_capacity;

    for(size_t i = 0; i < old_capacity; i++)
    {
        entry_t* entry = old_entries[i];
        while (entry != 0)
        {
            __hash_table_insert(ptable, entry->key, entry->value);
            entry = entry->next_entry;
        }
    }
}

hash_table_t __hash_table_create(size_t key_size, size_t value_size)
{
    hash_table_data_t* ptable = (hash_table_data_t*)malloc(sizeof(hash_table_data_t));
    ptable->capacity = DEFAULT_SIZE;
    ptable->key_size = key_size;
    ptable->value_size = value_size;
    ptable->length = 0;
    ptable->entries = malloc(sizeof(entry_t*) * DEFAULT_SIZE);

    for(size_t i = 0; i < DEFAULT_SIZE; i++)
        ptable->entries[i] = 0;

    return (hash_table_t)ptable;
}

void hash_table_destroy(hash_table_t hash_table)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    for(size_t i = 0; i < ptable->capacity; i++)
        if(ptable->entries[i])
            free_entry(ptable->entries[i]);
    free(ptable->entries);
    free(ptable);
}

void __hash_table_insert(hash_table_t hash_table, void* key_ptr, void* value_ptr)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    uint64_t slot = hash(key_ptr, ptable->key_size) % ptable->capacity;
    
    entry_t* entry = ptable->entries[slot];

    if(entry == 0)
    {
        ptable->entries[slot] = make_pair(ptable, key_ptr, value_ptr);
        goto end;
    }

    entry_t* prev;
    while(entry != 0)
    {
        if(memcmp(entry->key, key_ptr, ptable->key_size) == 0)
        {
            free(entry->value);
            entry->value = malloc(ptable->value_size);
            memcpy(entry->value, value_ptr, ptable->value_size);
            goto end;
        }

        prev = entry;
        entry = prev->next_entry;
    }

    prev->next_entry = make_pair(ptable, key_ptr, value_ptr);

    end:
    ptable->length++;
    if(ptable->length == ptable->capacity)
        rehash(ptable, ptable->capacity * 2);
}

void* __hash_table_at(hash_table_t hash_table, void* key_ptr)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    uint64_t slot = hash(key_ptr, ptable->key_size) % ptable->capacity;
    
    entry_t* entry = ptable->entries[slot];

    if(entry == 0)
        return 0;

    while(entry != 0)
        if(memcmp(entry->key, key_ptr, ptable->key_size) == 0)
            return entry->value;
        else
            entry = entry->next_entry;
    
    return 0;
}

void __hash_table_erase(hash_table_t hash_table, void* key_ptr)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    uint64_t slot = hash(key_ptr, ptable->key_size) % ptable->capacity;

    entry_t* entry = ptable->entries[slot];
    if(entry == 0)
        return;

    entry_t* prev;
    size_t idx = 0;
    while(entry != 0)
    {
        if(memcmp(entry->key, key_ptr, ptable->key_size) == 0)
        {
            // first item and no next entry
            if(entry->next_entry == 0 && idx == 0)
                ptable->entries[slot] = 0;

            // first item with a next entry
            if(entry->next_entry != 0 && idx == 0)
                ptable->entries[slot] = entry->next_entry;

            // last item
            if(entry->next_entry == 0 && idx != 0)
                prev->next_entry = 0;

            // middle item
            if(entry->next_entry != 0 && idx != 0)
                prev->next_entry = entry->next_entry;

            free(entry->key);
            free(entry->value);
            free(entry);

            ptable->length--;
            return;
        }

        prev = entry;
        entry = prev->next_entry;
        idx++;
    }
}

size_t hash_table_size(hash_table_t hash_table)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    return ptable->length;
}

bool hash_table_empty(hash_table_t hash_table)
{
    hash_table_data_t* ptable = get_ptable(hash_table);
    return ptable->length == 0;
}