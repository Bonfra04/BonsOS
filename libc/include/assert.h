#pragma once

#include <stdnoreturn.h>

#ifdef NDEBUG
    #define assert(expression) ((void)0)
#else
    #ifdef __cplusplus
    extern "C" {
    #endif

    noreturn void _assert(const char* expression, const char* file, int line);

    #ifdef __cplusplus
    }
    #endif

    #define assert(expression) ((expression) ? ((void)0) : _assert(#expression, __FILE__, __LINE__))
#endif
