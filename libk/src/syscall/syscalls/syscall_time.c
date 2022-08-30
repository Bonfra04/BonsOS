#include <timers/time.h>
#include <stdint.h>

uint64_t syscall_time()
{
    return time_sinceepoch();
}
