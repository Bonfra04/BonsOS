#include <time.h>

#include <syscalls.h>

time_t time(time_t* timer)
{
    time_t t = sys_time();
    if(timer)
        *timer = t;
    return t;
}
