#include <io/tty.h>
#include <io/uart.h>

#include <log.h>

#include <stdio.h>
#include <stdarg.h>

void __kernel_log(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buff[1024];
    vsnprintf(buff, sizeof(buff), format, args);

    va_end(args);

    tty_print(buff, false);
    uart_puts(buff);
}