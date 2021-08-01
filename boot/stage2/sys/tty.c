#include "tty.h"
#include "../lib/string.h"
#include "realmode.h"
#include "../lib/stdarg.h"
#include "../lib/stdlib.h"

static int vsnprintf(char* buf, size_t n, const char* format, va_list args);

static void addchar(char* buf, size_t n, int* result, char ch)
{
    if(n > 0 && *result <= n)
        buf[*result] = ch;
    (*result)++;
}

static void addstring(char* buf, size_t n, int* result, char* str)
{
    char ch;
    while(ch = *str++)
        addchar(buf, n, result, ch);
}

void tty_putchar(char c)
{
    rm_regs_t regs;
    regs.eax = 0x0E << 8 | c;
    rm_int(0x10, &regs, &regs);

    if(c == '\n')
        tty_putchar('\r');
}

void tty_print(const char *s)
{
    size_t len = strlen(s);
    for(size_t i = 0; i < len; i++)
        tty_putchar(s[i]);
}

int tty_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[512];
    int result = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    tty_print(buffer);

    return result;
}

static  int vsnprintf(char* buf, size_t n, const char* format, va_list args)
{
    n--;
    const char* fptr = format;
    int result = 0;

    char ch;
    while((ch = *fptr++) != 0)
    {
        if(ch == '%')
        {
            ch = *fptr++;
            switch(ch)
            {
                case 'l':
                {
                    ch = *fptr++;
                    switch(ch)
                    {
                        case 'l':
                        {
                            ch = *fptr++;
                            switch(ch)
                            {
                                case 'i':
                                case 'd':
                                {
                                    long long arg = va_arg(args, long long);
                                    char buff[32];
                                    lltoa(arg, buff, 10);
                                    addstring(buf, n, &result, buff);
                                    break;
                                }

                                case 'u':
                                {
                                    unsigned long long arg = va_arg(args, unsigned long long);
                                    char buff[32];
                                    ulltoa(arg, buff, 10);
                                    addstring(buf, n, &result, buff);
                                    break;
                                }

                                case 'x':
                                {
                                    unsigned long long arg = va_arg(args, unsigned long long);
                                    char buff[32];
                                    ulltoa(arg, buff, 16);
                                    addstring(buf, n, &result, buff);
                                    break;
                                }

                                case 'X':
                                {
                                    unsigned long long arg = va_arg(args, unsigned long long);
                                    char buff[32];
                                    ulltoa(arg, buff, 16);
                                    strtoupper(buff);
                                    addstring(buf, n, &result, buff);
                                    break;
                                }

                                case 'n':
                                {
                                    long long* arg = va_arg(args, long long*);
                                    *arg = result;
                                    break;
                                }
                            }
                            break;
                        }

                        case 'i':
                        case 'd':
                        {
                            long arg = va_arg(args, long);
                            char buff[32];
                            ltoa(arg, buff, 10);
                            addstring(buf, n, &result, buff);
                            break;
                        }

                        case 'u':
                        {
                            unsigned long arg = va_arg(args, unsigned long);
                            char buff[32];
                            ultoa(arg, buff, 10);
                            addstring(buf, n, &result, buff);
                            break;
                        }

                        case 'x':
                        {
                            unsigned long arg = va_arg(args, unsigned long);
                            char buff[32];
                            ultoa(arg, buff, 16);
                            addstring(buf, n, &result, buff);
                            break;
                        }

                        case 'X':
                        {
                            unsigned long arg = va_arg(args, unsigned long);
                            char buff[32];
                            ultoa(arg, buff, 16);
                            strtoupper(buff);
                            addstring(buf, n, &result, buff);
                            break;
                        }

                        case 'n':
                        {
                            long* arg = va_arg(args, long*);
                            *arg = result;
                            break;
                        }
                    }
                    break;
                }

                case 'i':
                case 'd':
                {
                    int arg = va_arg(args, int);
                    char buff[16];
                    itoa(arg, buff, 10);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'u':
                {
                    unsigned int arg = va_arg(args, unsigned int);
                    char buff[16];
                    uitoa(arg, buff, 10);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'o':
                {
                    unsigned int arg = va_arg(args, unsigned int);
                    char buff[16];
                    uitoa(arg, buff, 8);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'x':
                {
                    unsigned int arg = va_arg(args, unsigned int);
                    char buff[16];
                    uitoa(arg, buff, 16);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'X':
                {
                    unsigned int arg = va_arg(args, unsigned int);
                    char buff[16];
                    uitoa(arg, buff, 16);
                    strtoupper(buff);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'c':
                {
                    char arg = (char)va_arg(args, unsigned int);
                    addchar(buf, n, &result, arg);
                    break;
                }

                case 's':
                {
                    char* arg = va_arg(args, char*);
                    addstring(buf, n, &result, arg);
                    break;
                }

                case 'p':
                {
                    void* arg = va_arg(args, void*);
                    char buff[16];
                    ultoa((unsigned long)arg, buff, 16);
                    addstring(buf, n, &result, buff);
                    break;
                }

                case 'n':
                {
                    int* arg = va_arg(args, int*);
                    *arg = result;
                    break;
                }

                default:
                    fptr--;
                case '%':
                    addchar(buf, n, &result, '%');
                    break;
            }
        }
        else
            addchar(buf, n, &result, ch);
    }
    buf[result] = '\0';

    return result;
}