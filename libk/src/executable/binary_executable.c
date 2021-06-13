#include <executable/executable.h>
#include <stddef.h>
#include <schedule/scheduler.h>

void run_binary_executable(void* address, size_t num_pages)
{
    create_process(address, num_pages);
}