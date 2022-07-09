#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int sprintf(char* buf, const char* format, ...);
int vsprintf(char* buf, const char* format, va_list args);
int snprintf(char* buf, size_t n, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list args);

#ifdef __cplusplus
}
#endif
