#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum flag
{
    FLAG_NONE = 0,
    FLAG_LEFT = 1,
    FLAG_SIGN = 2,
    FLAG_SPACE = 4,
    FLAG_ALT = 8,
    FLAG_ZERO = 16,
} flag_t;

typedef enum length
{
    LEN_NONE,
    LEN_hh,
    LEN_h,
    LEN_l,
    LEN_ll,
    LEN_j,
    LEN_z,
    LEN_t,
} length_t;

typedef enum specifier
{
    SPEC_c,
    SPEC_s,
    SPEC_d,
    SPEC_o,
    SPEC_x,
    SPEC_X,
    SPEC_u,
    SPEC_n,
    SPEC_p,
} specifier_t;

typedef struct printer
{
    bool (*print)(void* where, char what);
    void* where;
} printer_t;

static bool printer_buffer(char** where, char what)
{
    **where = what;
    (*where)++;
    return true;
}

static int printer_stream(FILE* where, char what)
{
    return fputc(what, where) != EOF;
}

static int vsnprintf_internal(printer_t* printer, size_t n, const char* format, va_list args)
{
    // TODO: implement flags -+0

    n--;
    int result = 0;

    while(*format != '\0')
    {
        char ch = *format++;

        if(ch != '%')
        {
            result += printer->print(printer->where, ch);
            continue;
        }

        flag_t flags = FLAG_NONE;
        while(1)
            if(*format == '-')
                flags |= FLAG_LEFT, format++;
            else if(*format == '+')
                flags |= FLAG_SIGN, format++;
            else if(*format == ' ')
                flags |= FLAG_SPACE, format++;
            else if(*format == '#')
                flags |= FLAG_ALT, format++;
            else if(*format == '0')
                flags |= FLAG_ZERO, format++;
            else
                break;

        int width = 0;
        if(*format == '*')
            width = va_arg(args, int), format++;
        else
            while(*format >= '0' && *format <= '9')
                width = width * 10 + *format++ - '0';
        if(width < 0)
            flags |= FLAG_LEFT, width = -width;

        int precision = 0;
        if(*format == '.')
        {
            format++;
            if(*format == '*')
                precision = va_arg(args, int), format++;
            else
                while(*format >= '0' && *format <= '9')
                    precision = precision * 10 + *format++ - '0';
            if(precision < 0)
                precision = 0;
        }
        else
            precision = -1;

        length_t length = LEN_NONE;
        if(*format == 'h' && *(format + 1) == 'h')
            length = LEN_hh, format += 2;
        else if(*format == 'h')
            length = LEN_h, format++;
        else if(*format == 'l' && *(format + 1) == 'l')
            length = LEN_ll, format += 2;
        else if(*format == 'l')
            length = LEN_l, format++;
        else if(*format == 'j')
            length = LEN_j, format++;
        else if(*format == 'z')
            length = LEN_z, format++;
        else if(*format == 't')
            length = LEN_t, format++;

        specifier_t spec;
        if(*format == 'c')
            spec = SPEC_c, format++;
        else if(*format == 's')
            spec = SPEC_s, format++;
        else if(*format == 'd' || *format == 'i')
            spec = SPEC_d, format++;
        else if(*format == 'o')
            spec = SPEC_o, format++;
        else if(*format == 'x')
            spec = SPEC_x, format++;
        else if(*format == 'X')
            spec = SPEC_X, format++;
        else if(*format == 'u')
            spec = SPEC_u, format++;
        else if(*format == 'n')
            spec = SPEC_n, format++;
        else if(*format == 'p')
            spec = SPEC_p, format++;
        else
            continue;

        char buffer[1024]; // TODO: better calculate this size dinamically
        memset(buffer, '\0', sizeof(buffer));

        switch(spec)
        {
            case SPEC_c:
            {
                if(length == LEN_l)
                {
                    // TODO: wchar_t
                }
                else
                {
                    unsigned char arg = (unsigned char)va_arg(args, int);
                    buffer[0] = arg;
                }
            }
            break;

            case SPEC_s:
            {
                if(length == LEN_l)
                {
                    // TODO: wchar_t*
                }
                else
                {
                    char* arg = va_arg(args, char*);
                    if(precision != -1)
                        arg[precision] = '\0';
                    if(width > 0)
                        strncpy(buffer, arg, width);
                    else
                        strcpy(buffer, arg);
                }
            }
            break;

            case SPEC_d:
            {
                long long num;
                if(length == LEN_hh)
                    num = (signed char)va_arg(args, int);
                else if(length == LEN_h)
                    num = (short)va_arg(args, int);
                else if(length == LEN_l)
                    num = va_arg(args, long);
                else if(length == LEN_ll)
                    num = va_arg(args, long long);
                else if(length == LEN_j)
                    num = va_arg(args, intmax_t);
                else if(length == LEN_z)
                    num = va_arg(args, size_t);
                else if(length == LEN_t)
                    num = va_arg(args, ptrdiff_t);
                else
                    num = va_arg(args, int);

                lltoa(num, buffer, 10);
                size_t len = strlen(buffer);
                if(precision != -1 && precision > len)
                {
                    memcpy(buffer + (precision - len), buffer, len);
                    memset(buffer, '0', precision - len);
                }
                if(precision == 0 && num == 0)
                    buffer[0] = '\0';
            }
            break;

            case SPEC_o: case SPEC_x: case SPEC_X: case SPEC_u:
            {
                unsigned long long num;
                if(length == LEN_hh)
                    num = (unsigned char)va_arg(args, unsigned int);
                else if(length == LEN_h)
                    num = (unsigned short)va_arg(args, unsigned int);
                else if(length == LEN_l)
                    num = va_arg(args, unsigned long);
                else if(length == LEN_ll)
                    num = va_arg(args, unsigned long long);
                else if(length == LEN_j)
                    num = va_arg(args, uintmax_t);
                else if(length == LEN_z)
                    num = va_arg(args, size_t);
                else if(length == LEN_t)
                    num = va_arg(args, ptrdiff_t);
                else
                    num = va_arg(args, unsigned int);

                if(spec == SPEC_o && flags & FLAG_ALT)
                    buffer[0] = '0';

                int base = spec == SPEC_o ? 8 : spec == SPEC_X || spec == SPEC_x ? 16 : 10;
                ulltoa(num, buffer + (spec == SPEC_o && flags & FLAG_ALT), base);
                size_t len = strlen(buffer);
                if(precision != -1 && precision > len)
                {
                    memmove(buffer + (precision - len), buffer, len);
                    memset(buffer, '0', precision - len);
                    len += precision - len;
                }

                if((spec == SPEC_X || spec == SPEC_x) && num != 0 && flags & FLAG_ALT)
                {
                    memmove(buffer + 2, buffer, len);
                    buffer[0] = '0';
                    buffer[1] = 'x';
                }

                if(spec == SPEC_X)
                    strtoupper(buffer);

                if(precision == 0 && num == 0)
                    buffer[0] = '\0';
            }
            break;

            case SPEC_p:
            {
                void* arg = va_arg(args, void*);
                buffer[0] = '0';
                buffer[1] = 'x';
                ulltoa((unsigned long long)arg, buffer + 2, 16);
                strtoupper(buffer + 2);
            }
            break;

            case SPEC_n:
            {
                if(length == LEN_hh)
                    *va_arg(args, signed char*) = (signed char)result;
                else if(length == LEN_h)
                    *va_arg(args, short*) = (short)result;
                else if(length == LEN_l)
                    *va_arg(args, long*) = (long)result;
                else if(length == LEN_ll)
                    *va_arg(args, long long*) = (long long)result;
                else if(length == LEN_j)
                    *va_arg(args, intmax_t*) = (intmax_t)result;
                else if(length == LEN_z)
                    *va_arg(args, size_t*) = (size_t)result;
                else if(length == LEN_t)
                    *va_arg(args, ptrdiff_t*) = (ptrdiff_t)result;
                else
                    *va_arg(args, int*) = (int)result;
            }
            break;
        }

        {
            char* buf = buffer;
            while(*buf)
                result += printer->print(printer->where, *buf++);
        }
    }

    printer->print(printer->where, '\0');
    return result;
}

int sprintf(char* buf, const char* format, ...)
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
    printer_t printer;
    printer.print = printer_buffer;
    printer.where = &buf;
    return vsnprintf_internal(&printer, n, format, args);
}

int fprintf(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

int printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(stdout, format, args);
    va_end(args);
    return result;
}

int vprintf(const char * format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}

int vfprintf(FILE* stream, const char* format, va_list arg)
{
    printer_t printer;
    printer.print = printer_stream;
    printer.where = stream;
    return vsnprintf_internal(&printer, 0, format, arg);
}
