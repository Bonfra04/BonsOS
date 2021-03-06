#pragma once

#include <syscall/syscall.h>

/**
 * @brief terminates the current thread, if no more thread are running call syscall_process_terminate
 * @param[in] params ignored
 * @return ignored
 */
uint64_t syscall_thread_terminate(const syscall_parameter_t* params);

/**
 * @brief terminates all threads of the current process and frees its memory
 * @param[in] params ignored
 * @return ignored
 */
uint64_t syscall_process_terminate(const syscall_parameter_t* params);

/**
 * @brief create a process for an executable
 * @param[in] r8 pointer to the string contianing the path of the executable
 * @param[in] r9 executable format
 * @return pid (0 on error)
 */
uint64_t syscall_process_execute(const syscall_parameter_t* params);

/**
 * @brief reads the content of an opened file to a struct
 * @param[in] r8 pointer to the file handler
 * @param[in] r9 buffer to store file content
 * @param[in] r10 amout of bytes to read
 */
uint64_t syscall_read_file(const syscall_parameter_t* params);

/**
 * @brief reads the content of an opened file to a struct
 * @param[in] r8 pointer to the file handler
 * @param[in] r9 buffer of data to copy in the file
 * @param[in] r10 amout of bytes to write
 */
uint64_t syscall_write_file(const syscall_parameter_t* params);

/**
 * @brief set the absolute position of a file
 * @param[in] r8 pointer to the file handler
 * @param[in] r9 absolute position to seek to
 * @return ignored
 */
uint64_t syscall_seek_file(const syscall_parameter_t* params);

/**
 * @brief retrieve the absolute position of a file
 * @param[in] r8 pointer to the file handler
 * @return the absolute position of the file
 */
uint64_t syscall_tell_file(const syscall_parameter_t* params);

/**
 * @brief opens a file
 * @param[in] r8 pointer to a string containing the file name
 * @param[in] r9 pointer to a string contianing the mode
 * @param[out] r10 pointer to the file handler
 * @return ignored
 */
uint64_t syscall_file_open(const syscall_parameter_t* params);

/**
 * @brief closes a file
 * @param[in] r8 pointer to the file handler to close
 * @return success
 */
uint64_t syscall_file_close(const syscall_parameter_t* params);

/**
 * @brief remove a file
 * @param[in] r8 pointer to a string containing the file name
 * @return success
 */
uint64_t syscall_file_remove(const syscall_parameter_t* params);

/**
 * @brief maps a physical memory area to a virtual one
 * @param[in] r8 physicall memory address
 * @param[in] r9 size of the memory area in bytes
 * @return virtual memory address
 */
uint64_t syscall_map_mem(const syscall_parameter_t* params);

/**
 * @brief send a message to another process
 * @param[in] r8 pid of the process to send the message to
 * @param[in] r9 pointer to the message
 * @return ignored
 */
uint64_t syscall_msg_send(const syscall_parameter_t* params);

/**
 * @brief get a message from the queue, if no message is available zero is returned
 * @param[out] r8 pointer to the message to populate
 * @return pid of the sender
 */
uint64_t syscall_msg_fetch(const syscall_parameter_t* params);

/**
 * @brief retrieves the absolute position of the mouse
 * @param[out] r8 x coordinate
 * @param[out] r9 y coordinate
 * @return ignored
 */
uint64_t syscall_get_mouse(const syscall_parameter_t* params);

/**
 * @brief checks if a key is pressed
 * @param[in] r8 key to check
 * @return true if key is pressed, otherwise false
 */
uint64_t syscall_is_key_pressed(const syscall_parameter_t* params);
