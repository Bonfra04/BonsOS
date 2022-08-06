#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OPEN_READ 0
#define OPEN_WRITE 1
#define OPEN_APPEND 2

#define SYSCALL_OPEN_FILE 0
#define SYSCALL_CLOSE_FILE 1
#define SYSCALL_READ_FILE 2
#define SYSCALL_WRITE_FILE 3
#define SYSCALL_MAP_MEM 4
#define SYSCALL_THREAD_EXIT 5
#define SYSCALL_PROCESS_EXIT 6
#define SYSCALL_EXEC 7

uint64_t sys(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

int sys_open_file(const char* path, int mode);
bool sys_close_file(int fd);
size_t sys_read_file(int fd, char* buff, size_t count);
size_t sys_write_file(int fd, const char* buff, size_t count);
void* sys_map_mem(void* ph_addr, size_t size);
void sys_thread_exit();
void sys_process_exit();
void sys_exec(const char* path, const char** argv);
