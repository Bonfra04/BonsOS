#include <stdlib.h>

int abs(int n)
{
    return n < 0 ? -n : n;
}

div_t div(int numer, int denom)
{
    return (div_t){ numer / denom, numer % denom };
}

long labs (long n)
{
    return n < 0 ? -n : n;
}

ldiv_t ldiv(long numer, long denom)
{
    return (ldiv_t){ numer / denom, numer % denom };
}

long long llabs(long long n)
{
    return n < 0 ? -n : n;
}

lldiv_t lldiv(long long numer, long long denom)
{
    return (lldiv_t){ numer / denom, numer % denom };
}