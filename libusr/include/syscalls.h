#pragma once

#include <stdint.h>

typedef enum executable_format
{
    BINARY,
    ELF,
    PE,
} executable_format_t;

uint64_t run_executable(const char* path, int argc, char* argv[], executable_format_t format);
