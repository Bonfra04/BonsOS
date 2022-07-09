#include <assert.h>

#ifdef KERNEL_BUILD

#include <panic.h>

void _assert(const char* expression, const char* file, int line)
{
    __kernel_panic("Assertion failed: %s (%s:%d)\n", expression, file, line);    
}

#else

// TODO: implement
void _assert(const char* expression, const char* file, int line)
{
    (void)expression, (void)file, (void)line;
    // fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", expression, file, line);
    // abort()
    for(;;);
}

#endif