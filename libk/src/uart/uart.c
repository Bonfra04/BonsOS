#include <uart/uart.h>
#include <stdarg.h>
#include <stdio.h>

void uart_printf(char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buff[512], *s = (char*)&buff;
    vsprintf(buff, format, args);
    va_end(args);
    while(*s) {
        if(*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

char* uart_gets(char* str)
{
    char* ptr = str;
    do {
        *ptr = uart_getc();
    } while(*ptr++ != '\n');
    *--ptr = '\0';
    return str;
}