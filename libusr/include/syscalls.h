#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum executable_format
{
    BINARY,
    ELF,
    PE,
} executable_format_t;

uint64_t run_executable(const char* path, int argc, char* argv[], executable_format_t format);

void* map_mem(void* ph_mem, size_t size);
