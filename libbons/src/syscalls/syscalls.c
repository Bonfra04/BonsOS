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

void sys_process_exit()
{
    sys(SYSCALL_PROCESS_EXIT, 0, 0, 0);
}

uint64_t sys_exec(const char* path, const char** argv, const char** env)
{
    return sys(SYSCALL_EXEC, (uint64_t)path, (uint64_t)argv, (uint64_t)env);
}

char* sys_getcwd(char* buff, size_t size)
{
    return (char*)sys(SYSCALL_GETCWD, (uint64_t)buff, (uint64_t)size, 0);
}

int sys_setcwd(char* buff)
{
    return (int)sys(SYSCALL_SETCWD, (uint64_t)buff, 0, 0);
}

bool sys_delete_file(const char* path)
{
    return (bool)sys(SYSCALL_DELETE_FILE, (uint64_t)path, 0, 0);
}

void sys_sched_yield()
{
    sys(SYSCALL_SCHED_YIELD, 0, 0, 0);
}

uint64_t sys_time()
{
    return sys(SYSCALL_TIME, 0, 0, 0);
}

bool sys_seek_file(int fd, uint64_t offset)
{
    return (bool)sys(SYSCALL_SEEK_FILE, (uint64_t)fd, (uint64_t)offset, 0);
}

uint64_t sys_tell_file(int fd)
{
    return sys(SYSCALL_TELL_FILE, (uint64_t)fd, 0, 0);
}

void sys_raise_signal(uint64_t tid, uint64_t signal)
{
    sys(SYSCALL_RAISE_SIGNAL, (uint64_t)tid, (uint64_t)signal, 0);
}
