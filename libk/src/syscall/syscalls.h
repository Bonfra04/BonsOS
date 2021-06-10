#pragma once

#include <syscall/syscall.h>

void syscall_thread_terminate(const syscall_parameter_t* params);
void syscall_process_terminate(const syscall_parameter_t* params);