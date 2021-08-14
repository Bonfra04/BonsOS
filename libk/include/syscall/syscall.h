#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct syscall_parameters
{
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t arg4;
} __attribute__((packed)) syscall_parameter_t;

typedef uint64_t(*syscall_t)(const syscall_parameter_t*);

/**
 * @brief initializie system calls
 */
void syscall_init();

/**
 * @brief register a syscall
 * @param[in] id the id of the system call (value of rax while calling it)
 * @param[in] systemcall function pointer to the system call handler
 */
void syscall_register(size_t id, syscall_t systemcall);