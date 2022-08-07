#pragma once

#include <stddef.h>

typedef enum executable_format
{
    EXEC_ELF
} executable_format_t;

typedef struct executable
{
    executable_format_t format;
    void* base_address;
    size_t num_pages;
    void* entry_point;
    char* fullpath;
} executable_t;

/**
 * @brief loads an executable from disk automatically detecting the format
 * @param[in] path of the executable
 */
executable_t* executable_load(const char* path);

/**
 * @brief unloads an executable from memory
 * @param[in] executable to unload
 */
void executable_unload(const executable_t* executable);
