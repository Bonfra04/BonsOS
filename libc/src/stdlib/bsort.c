#include <stdlib.h>

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*))
{
    const char* p;
    size_t n;

    for (p = (const char*)base, n = num; 0 < n; )
    {
        const size_t pivot = n >> 1;
        const char* const q = p + size * pivot;
        const int val = (*compar)(key, q);

        if (val < 0)
            n = pivot;
        else if (val == 0)
            return (void*)q;
        else
        {
            p = q + size;
            n -= pivot + 1;
        }
    }

    return (NULL);
}