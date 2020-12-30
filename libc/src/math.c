#include <math.h>

double pow(double base, int exponent)
{
    double res = 1;
    while (exponent--)
        res *= base;

    return res;
}