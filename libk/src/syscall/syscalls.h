#pragma once

#include <stddef.h>
#include <stdbool.h>

int syscall_open_file(const char* path, int mode);
bool syscall_close_file(int fd);
int syscall_read_file(int fd, char* buff, size_t count);
int syscall_write_file(int fd, char* buff, size_t count);
