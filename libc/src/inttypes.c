#include <inttypes.h>
#include <stdlib.h>

intmax_t imaxabs(intmax_t n)
{
    return n < 0 ? -n : n;
}

imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
    return (imaxdiv_t){ numer / denom, numer % denom };
}

intmax_t strtoimax(const char* str, char** endptr, int base)
{
    return strtoll(str, endptr, base);
}

uintmax_t strtoumax(const char* str, char** endptr, int base)
{
    return strtoull(str, endptr, base);
}
