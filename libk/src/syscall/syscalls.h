#pragma once

#include <syscall/syscall.h>

/**
 * @brief terminates the current thread, if no more thread are running call syscall_process_terminate
 * @param[in] params ignored
 */
void syscall_thread_terminate(const syscall_parameter_t* params);

/**
 * @brief terminates all threads of the current process and frees its memory
 * @param[in] params ignored
 */
void syscall_process_terminate(const syscall_parameter_t* params);

/**
 * @brief create a process for an executable
 * @param[in] r8 pointer to the string contianing the path of the executable
 * @param[in] r9 executable format
 */
void syscall_process_execute(const syscall_parameter_t* params);

/**
 * @brief reads the content of an opened file to a struct
 * @param[in] r8 pointer to the file handler
 * @param[in] r9 buffer to store file content
 * @param[in] r10 amout of bytes to read
 */
void syscall_read_file(const syscall_parameter_t* params);

/**
 * @brief reads the content of an opened file to a struct
 * @param[in] r8 pointer to the file handler
 * @param[in] r9 buffer of data to copy in the file
 * @param[in] r10 amout of bytes to write
 */
void syscall_write_file(const syscall_parameter_t* params);