#pragma once

#include <stddef.h>

typedef size_t time_t;

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#ifdef __cplusplus
extern "C" {
#endif

time_t time(time_t* timer);
struct tm* localtime(const time_t* timer);
char* asctime(const struct tm* time_ptr);
char* ctime(const time_t* timer);

#ifdef __cplusplus
}
#endif
