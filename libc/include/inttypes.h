#pragma once

#include <stdint.h>

typedef struct
{
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

intmax_t imaxabs(intmax_t n);
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
intmax_t strtoimax(const char* str, char** endptr, int base);
uintmax_t strtoumax(const char* str, char** endptr, int base);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif