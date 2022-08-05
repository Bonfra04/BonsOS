#include <syscalls.h>

int sys_open_file(const char* path, int mode)
{
    return (int)sys(SYSCALL_OPEN_FILE, (uint64_t)path, (uint64_t)mode, 0);
}

bool sys_close_file(int fd)
{
    return (bool)sys(SYSCALL_CLOSE_FILE, (uint64_t)fd, 0, 0);
}

size_t sys_read_file(int fd, char* buff, size_t count)
{
    return sys(SYSCALL_READ_FILE, (uint64_t)fd, (uint64_t)buff, (uint64_t)count);
}

size_t sys_write_file(int fd, const char* buff, size_t count)
{
    return sys(SYSCALL_WRITE_FILE, (uint64_t)fd, (uint64_t)buff, (uint64_t)count);
}

void* sys_map_mem(void* ph_addr, size_t size)
{
    return (void*)sys(SYSCALL_MAP_MEM, (uint64_t)ph_addr, (uint64_t)size, 0);
}

void sys_thread_exit()
{
    sys(SYSCALL_THREAD_EXIT, 0, 0, 0);
}
