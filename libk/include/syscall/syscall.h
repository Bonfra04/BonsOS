#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct syscall_parameters
{
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
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