#include <panic.h>
#include <device/tty.h>
#include <stdarg.h>
#include <stdio.h>
#include <x86/cpu.h>

void kenrel_panic(const char* format, ...)
{
    tty_set_textcolor(0xFFFFFF00, 0xFFFF0000); // yellow, red

    char buffer[8 * 1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, buffer);
    for(;;);
}