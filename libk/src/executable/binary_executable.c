#include <executable/executable.h>
#include <stddef.h>
#include <schedule/scheduler.h>

size_t run_binary_executable(void* address, int argc, char* argv[], size_t num_pages)
{
    // does not work cause of unknown memory size
    return create_process(address, argc, argv, num_pages);
}