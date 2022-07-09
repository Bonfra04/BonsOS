#include <io/uart.h>
#include <io/ports.h>

void uart_putc(char c)
{
    while (!(inportb(0x3FD) & (1 << 5)));
    outportb(0x3F8, c);
}

void uart_puts(const char* str)
{
    while(*str)
    {
        if(*str == '\n')
            uart_putc('\r');
        uart_putc(*str++);
    }
}

char uart_getc()
{
    while (!(inportb(0x3FD) & (1 << 0)));
    uint8_t c = inportb(0x3F8);
    if(c == '\r')
        return '\n';
    return c;
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
