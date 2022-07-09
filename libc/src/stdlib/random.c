#include <stdlib.h>

static unsigned int _seed = 1;

int rand(void)
{
    unsigned int next = _seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    _seed = next;

    return result;
}

void srand(unsigned int seed)
{
    _seed = seed;
}
