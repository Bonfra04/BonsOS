#include <uart/uart.h>
#include <stdarg.h>
#include <stdio.h>
#include <x86/ports.h>

void uart_putc(char c)
{
    while (!(inportb(0x3FD) & (1 << 5)));
    outportb(0x3F8, c);
}

char uart_getc()
{
    while (!(inportb(0x3FD) & (1 << 0)));
    uint8_t c = inportb(0x3F8);
    if(c == '\r')
        return '\n';
    return c;
}


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