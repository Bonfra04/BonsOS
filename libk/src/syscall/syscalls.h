#pragma once

#include <stddef.h>
#include <stdbool.h>

int syscall_open_file(const char* path, int mode);
bool syscall_close_file(int fd);
size_t syscall_read_file(int fd, char* buff, size_t count);
size_t syscall_write_file(int fd, char* buff, size_t count);
void* syscall_map_mem(void* ph_addr, size_t size);
void syscall_thread_exit();
void syscall_process_exit();
void syscall_exec(const char* path, const char** argv, const char** env);
char* syscall_getcwd(char* buff, size_t size);
int syscall_setcwd(char* buff);
bool syscall_delete_file(const char* path);
void syscall_sched_yield();
