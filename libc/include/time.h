#pragma once

#include <stddef.h>

typedef size_t time_t;

#ifdef __cplusplus
extern "C" {
#endif

time_t time(time_t* timer);

#ifdef __cplusplus
}
#endif
