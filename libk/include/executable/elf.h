#pragma once

#include <executable/executable.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief checks weather the given file is an ELF executable
 * @param[in] file_data to check
 */
bool elf_check(uint8_t* file_data);

/**
 * @brief populates an executable_t struct with the data from the given file_data
 * @param[in] file_data file to load
 * @param[in] num_pages size of the file in pages
 * @note assumes elf_check(file_data) has been called
 */
executable_t* elf_load(uint8_t* file_data, size_t num_pages);

/**
 * @brief unloads an executable_t struct from memory
 * @param[in] executable to unload
 * @note assumes executable->format == ELF
 */
void elf_unload(const executable_t* executable);
