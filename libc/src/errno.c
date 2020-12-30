#include <errno.h>

int errno_val = 0;

int* _errno(void)
{
    return &errno_val;
}

void _set_errno(int value)
{
    errno_val = value;
}

int _get_errno(void)
{
    return errno_val;
}