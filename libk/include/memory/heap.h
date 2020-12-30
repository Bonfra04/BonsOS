#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct heap_region
{
    uint64_t length;
    struct heap_region* next_region;
    struct heap_region* previous_region;
    bool free;
} heap_region_t;

typedef struct
{
    uint64_t base_address;
    uint64_t size;
    heap_region_t* first_region;
} heap_data_t;

heap_data_t heap_create(void* base_address, uint64_t size);
void heap_activate(heap_data_t* heap);

void* heap_malloc(size_t size);
void heap_free(void* address);
void* heap_realloc(void* address, size_t size);
void* heap_calloc(size_t amount, size_t size);