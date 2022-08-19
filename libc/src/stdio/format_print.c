#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

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

static bool printer_buffer(void* where_, char what)
{
    char** where = where_;
    **where = what;
    (*where)++;
    return true;
}

static bool printer_stream(void* where_, char what)
{
    FILE* where = where_;
    return fputc(what, where) != EOF;
}

typedef struct format_data
{
    flag_t flags;
    int width;
    int precision;
    length_t length;
    specifier_t specifier;
} format_data_t;

static int format_numeric(format_data_t format, printer_t* printer, const char* prefix, const char* buf)
{
    int printed_chars = 0;
    size_t len = strlen(buf);
    size_t offset = len;

    bool pad_left = format.flags & FLAG_LEFT;
    bool pad_zero = format.flags & FLAG_ZERO;
    bool use_sign = format.flags & FLAG_SIGN;
    bool use_space = format.flags & FLAG_SPACE;

    if(pad_zero && buf[0] != '-')
        if(use_sign)
        {
            if(printer->print(printer->where, '+'))
                printed_chars++, offset++;
        }
        else if(use_space)
        {
            if(printer->print(printer->where, ' '))
                printed_chars++, offset++;
        }

    size_t pref_len = prefix ? strlen(prefix) : 0;
    for(size_t i = 0; i < pref_len; i++)
        if(printer->print(printer->where, prefix[i]))
            printed_chars++, offset++;

    size_t prec = format.precision < 0 ? 0 : offset < format.precision ? format.precision - offset : 0;
    size_t pad = prec + offset < format.width ? format.width - (prec + offset) : 0;

    if(!pad_left)
        for(size_t i = 0; i < pad; i++)
            printed_chars += printer->print(printer->where, pad_zero ? '0' : ' ');

    if(!pad_zero && buf[0] != '-')
        if(use_sign)
        {
            if(printer->print(printer->where, '+'))
                printed_chars++;
        }
        else if(use_space)
        {
            if(printer->print(printer->where, ' '))
                printed_chars++;
        }

    for(size_t i = 0; i < prec; i++)
        printed_chars += printer->print(printer->where, '0');

    for(size_t i = 0; i < len; i++)
        printed_chars += printer->print(printer->where, buf[i]);
    
    if(pad_left)
        for(size_t i = 0; i < pad; i++)
            printed_chars += printer->print(printer->where, ' ');

    return printed_chars;
}

static void eval_format(format_data_t format, va_list args, printer_t* printer, int* printed_chars)
{
    switch(format.specifier)
    {
        case SPEC_c:
        {
            if(format.length == LEN_l)
            {
                // TODO: wchar_t
            }
            else
            {
                unsigned char arg = (unsigned char)va_arg(args, int);
                *printed_chars += printer->print(printer->where, arg);
            }
        }
        break;

        case SPEC_s:
        {
            if(format.length == LEN_l)
            {
                // TODO: wchar_t*
            }
            else
            {
                char* arg = va_arg(args, char*);
                size_t len = ullmin(strlen(arg), format.precision);
                size_t pad = len < format.width ? format.width - len : 0;

                if(format.flags & FLAG_LEFT)
                    for(size_t i = 0; i < pad; i++)
                        *printed_chars += printer->print(printer->where, ' ');
                for(size_t i = 0; i < len; i++)
                    *printed_chars += printer->print(printer->where, arg[i]);
                if(!(format.flags & FLAG_LEFT))
                    for(size_t i = 0; i < pad; i++)
                        *printed_chars += printer->print(printer->where, ' ');
            }
        }
        break;

        case SPEC_d:
        {
            long long num;
            if(format.length == LEN_hh)
                num = (signed char)va_arg(args, int);
            else if(format.length == LEN_h)
                num = (short)va_arg(args, int);
            else if(format.length == LEN_l)
                num = va_arg(args, long);
            else if(format.length == LEN_ll)
                num = va_arg(args, long long);
            else if(format.length == LEN_j)
                num = va_arg(args, intmax_t);
            else if(format.length == LEN_z)
                num = va_arg(args, size_t);
            else if(format.length == LEN_t)
                num = va_arg(args, ptrdiff_t);
            else
                num = va_arg(args, int);

            char buf[32];
            lltoa(num, buf, 10);
            *printed_chars += format_numeric(format, printer, NULL, buf);
        }
        break;

        case SPEC_o: case SPEC_x: case SPEC_X: case SPEC_u:
        {
            unsigned long long num;
            if(format.length == LEN_hh)
                num = (unsigned char)va_arg(args, unsigned int);
            else if(format.length == LEN_h)
                num = (unsigned short)va_arg(args, unsigned int);
            else if(format.length == LEN_l)
                num = va_arg(args, unsigned long);
            else if(format.length == LEN_ll)
                num = va_arg(args, unsigned long long);
            else if(format.length == LEN_j)
                num = va_arg(args, uintmax_t);
            else if(format.length == LEN_z)
                num = va_arg(args, size_t);
            else if(format.length == LEN_t)
                num = va_arg(args, ptrdiff_t);
            else
                num = va_arg(args, unsigned int);

            int base = 10;
            char* prefix = NULL;
            switch (format.specifier)
            {
                case SPEC_o: 
                    base = 8;
                    prefix = format.flags & FLAG_ALT ? "0" : NULL;
                    break;
                case SPEC_x:
                    base = 16;
                    prefix = format.flags & FLAG_ALT ? "0x" : NULL;
                    break;
                case SPEC_X:
                    base = 16;
                    prefix = format.flags & FLAG_ALT ? "0X" : NULL;
                    break;
                default:
                case SPEC_u: base = 10, prefix = NULL; break;
            } 

            char buf[32];
            ulltoa(num, buf, base);
            *printed_chars += format_numeric(format, printer, prefix, buf);
        }
        break;

        case SPEC_p:
        {
            void* arg = va_arg(args, void*);
            char buf[32];
            ulltoa((unsigned long long)arg, buf, 16);
            strtoupper(buf);
            *printed_chars += format_numeric(format, printer, "0x", buf);
        }
        break;
    }
}

static int vsnprintf_internal(printer_t* printer, size_t n, const char* format, va_list args)
{
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

        int precision = -1;
        if(*format == '.')
        {
            format++;
            if(*format == '*')
            {
                precision = va_arg(args, int), format++;
                if(precision < 0)
                    precision = 0;
            }
            else
            {
                precision = 0;
                while(*format >= '0' && *format <= '9')
                    precision = precision * 10 + *format++ - '0';
            }
        }

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

        specifier_t specifier;
        if(*format == 'c')
            specifier = SPEC_c, format++;
        else if(*format == 's')
            specifier = SPEC_s, format++;
        else if(*format == 'd' || *format == 'i')
            specifier = SPEC_d, format++;
        else if(*format == 'o')
            specifier = SPEC_o, format++;
        else if(*format == 'x')
            specifier = SPEC_x, format++;
        else if(*format == 'X')
            specifier = SPEC_X, format++;
        else if(*format == 'u')
            specifier = SPEC_u, format++;
        else if(*format == 'n')
            specifier = SPEC_n, format++;
        else if(*format == 'p')
            specifier = SPEC_p, format++;
        else
            continue;

        format_data_t format_data = {flags, width, precision, length, specifier};
        eval_format(format_data, args, printer, &result);
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
