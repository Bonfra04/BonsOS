#pragma once

#include <stddef.h>

typedef enum executable_format
{
    BINARY,
    ELF,
    PE,
} executable_format_t;

/**
 * @brief parse an executable file and enqueue a process for it
 * @param[in] path path of the executable
 * @param[in] privilege privilege of the process
 * @param[in] format executable format
 * @return pid (0 on error)
 */
size_t run_executable(const char* path, int argc, char* argv[], executable_format_t format);