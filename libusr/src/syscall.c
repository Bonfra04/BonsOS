#include <syscalls.h>

uint64_t sys(uint64_t rax, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t r12, uint64_t r13);

#define SYSCALL_RUN_EXECUTABLE 2
#define SYSCALL_MAP_MEM 10

uint64_t run_executable(const char* path, int argc, char* argv[], executable_format_t format)
{
    sys(SYSCALL_RUN_EXECUTABLE, path, format, argc, argv, 0);
}

void* map_mem(void* ph_mem, size_t size)
{
    return (void*)sys(10, ph_mem, size, 0, 0, 0);
}