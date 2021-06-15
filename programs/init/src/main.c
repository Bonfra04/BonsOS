#include <stdint.h>
#include <stddef.h>
#include <string.h>

extern void sys_fwrite(size_t file, void* buffer, size_t len);

void main()
{
    char* str = "Hello, World!";
    sys_fwrite(1, str, 14);
}