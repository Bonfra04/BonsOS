#include <panic.h>
#include <log.h>
#include <io/uart.h>
#include <io/tty.h>
#include <cpu.h>

#include <stdarg.h>
#include <stdio.h>

void __kernel_panic(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buff[1024];
    vsnprintf(buff, sizeof(buff), format, args);

    va_end(args);
    
    tty_set_textcolor(0xFFFF00, 0xFF0000);
    kernel_log(buff);
    
    for(;;)
    {
        cli();
        hlt();
    }
}