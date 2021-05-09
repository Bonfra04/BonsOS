#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

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

int sprintf(char* buf, const char* format, ... )
{
    va_list args;
    va_start(args, format);
    int result = vsprintf(buf, format, args);
    va_end(args);
    return result;
}

int vsprintf(char* buf, const char* format, va_list args)
{
    return vsnprintf(buf, 0, format, args);
}

int snprintf(char* buf, size_t n, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buf, n, format, args);
    va_end(args);
    return result;
}

int vsnprintf(char* buf, size_t n, const char* format, va_list args)
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

                case 'f':
                case 'F':
                {
                    float arg = (float)va_arg(args, double);
                    char buff[32];
                    ftoa(arg, buff, 7);
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

int sscanf(const char* s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsscanf(s, format, args);
    va_end(args);
    return result;  
}


int vsscanf(const char* s, const char *format, va_list args)
{
    int res = 0;

    char buff_a[strlen(s)];
    char* buff = buff_a;
    strcpy(buff, s);

    for(char* p = buff; *buff && *format; buff++)
    {
        while(*format == ' ')
            format++;

        while(*buff && *buff == ' ')
            buff++;

        p = buff;


        for(; *format; format++)
            switch (*format)
            {
            case ' ':
                break;
            case '%':
                if (!format[1])
                    return res;
                else switch (*++format)
                {
                case '%':
                    goto generic;

                case 'd':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        int* arg = va_arg(args, int*);
                        *arg = atoi(p);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'u':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        unsigned int* arg = va_arg(args, unsigned int*);
                        *arg = atoui(p);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'o':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        unsigned int* arg = va_arg(args, unsigned int*);
                        *arg = (unsigned int)strtol(p, 0, 8);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'x':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        unsigned int* arg = va_arg(args, unsigned int*);
                        *arg = (unsigned int)strtol(p, 0, 16);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'f':
                case 'e':
                case 'g':
                case 'a':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        float* arg = va_arg(args, float*);
                        *arg = atof(p);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'c':
                    {
                        char* arg = va_arg(args, char*);
                        *arg = *p;
                        res++;
                        format++;
                        goto stop;  
                    }

                case 's':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        char* arg = (char*)va_arg(args, char*);
                        strcpy(arg, p);
                        res++;
                        format++;
                        goto stop;  
                    }

                case 'p':
                    {
                        while(*buff && !isspace(*buff))
                            buff++;
                        p[buff - p] = 0;

                        unsigned long* arg = va_arg(args, unsigned long*);
                        *arg = strtoul(p, 0, 16);
                        res++;
                        format++;
                        goto stop;
                    }

                case 'n':
                    {
                        int* arg = va_arg(args, int*);
                        *arg = strlen(s) - strlen(buff);
                        buff--;
                        res++;
                        format++;
                        goto stop;  
                    }

                default:
                    return res;
                }
            generic:
            default:
                if(*format == *p)
                    p++;
                else
                    return res;
            }
        stop:
        continue;
    }

    return res;
}