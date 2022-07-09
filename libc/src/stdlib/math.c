#include <stdlib.h>

int abs(int n)
{
    return n < 0 ? -n : n;
}
long labs(long n)
{
    return n < 0l ? -n : n;
}
long long llabs(long long n)
{
    return n < 0ll ? -n : n;
}

div_t div(int numer, int denom)
{
    return (div_t){ numer / denom, numer % denom };
}

ldiv_t ldiv(long numer, long denom)
{
    return (ldiv_t){ numer / denom, numer % denom };
}

lldiv_t lldiv(long long numer, long long denom)
{
    return (lldiv_t){ numer / denom, numer % denom };
}
