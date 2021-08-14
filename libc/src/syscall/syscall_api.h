#pragma once

#include <stdint.h>

#ifdef KERNEL_BUILD
#define SYS_FWRITE(file, buffer, length) fsys_write_file(file, buffer, length)
#define SYS_FREAD(file, buffer, length) fsys_read_file(file, buffer, length)
#define SYS_FSEEK(file, offset) fsys_set_position(file, offset)
#define SYS_FTELL(file) fsys_get_position(file)
#define SYS_FOPEN(filename, mode, handle) *handle = fsys_open_file(filename, mode)
#define SYS_FCLOSE(file) fsys_close_file(file)
#define SYS_REMOVE(filename) fsys_delete_file(filename)
#else
#define SYS_FWRITE(file, buffer, length) sys(4, (uint64_t)file, (uint64_t)buffer, (uint64_t)length, 0, 0)
#define SYS_FREAD(file, buffer, length) sys(3, (uint64_t)file, (uint64_t)buffer, (uint64_t)length, 0, 0)
#define SYS_FSEEK(file, offset) sys(5, (uint64_t)file, (uint64_t)offset, 0, 0, 0)
#define SYS_FTELL(file) sys(6, (uint64_t)file, 0, 0, 0, 0)
#define SYS_FOPEN(filename, mode, handle) sys(7, (uint64_t)filename, (uint64_t)mode, (uint64_t)handle, 0, 0)
#define SYS_FCLOSE(file) sys(8, (uint64_t)file, 0, 0, 0, 0)
#define SYS_REMOVE(filename) sys(9, (uint64_t)filename, 0, 0, 0, 0)
#endif

uint64_t sys(uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
