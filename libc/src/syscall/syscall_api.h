#pragma once

#include <stdint.h>

#ifdef KERNEL_BUILD
#define SYS_FWRITE(file, buffer, length) fsys_write_file(file, buffer, length)
#define SYS_FREAD(file, buffer, length) fsys_read_file(file, buffer, length)
#define SYS_FSEEK(file, offset) fsys_set_position(file, offset)
#define SYS_FTELL(file) fsys_get_position(file)
#define SYS_FOPEN(filename, mode, handle) *handle = fsys_open_file(filename, mode)
#define SYS_FCLOSE(file) fsys_close_file(file)
#else
#define SYS_FWRITE(file, buffer, length) sys(4, file, buffer, length, 0, 0)
#define SYS_FREAD(file, buffer, length) sys(3, file, buffer, length, 0, 0)
#define SYS_FSEEK(file, offset) sys(5, file, offset, 0, 0, 0)
#define SYS_FTELL(file) sys(6, file, 0, 0, 0, 0)
#define SYS_FOPEN(filename, mode, handle) sys(7, filename, mode, handle, 0, 0)
#define SYS_FCLOSE(file) sys(8, file, 0, 0, 0, 0)
#endif

uint64_t sys(uint64_t rax, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t r11, uint64_t r12);
