#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct scanner
{
    bool (*scan)(void* where, char* what);
    void* where;
} scanner_t;

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
    SPEC_i,
    SPEC_u,
    SPEC_o,
    SPEC_x,
    SPEC_p,
    SPEC_n,
    SPEC_SET,
    SPEC_NSET,
} specifier_t;

static bool scanner_buffer(void* where_, char* what)
{
    char** where = where_;
    if (**where == '\0')
        return false;
    *what = **where;
    (*where)++;
}

static bool scanner_stream(void* where_, char* what)
{
    FILE* where = where_;
    int c = fgetc(where);
    if (c == EOF)
        return false;
    *what = c;
}

typedef struct format_data
{
    bool ignore;
    int width;
    length_t length;
    specifier_t specifier;
    const char* charset;
} format_data_t;

static void eval_format(format_data_t format, va_list args, scanner_t* scanner, int* matched)
{
    // TODO: implement
}

static int vsscanf_internal(scanner_t* scanner, const char* format, va_list args)
{
    int result = 0;
    bool anyread = false;

    while(*format != '\0')
    {
        char chr, fmt = *format++;
        if(!scanner->scan(scanner->where, &chr))
            return anyread ? EOF : result;
        anyread = true;

        if(fmt != '%')
        {
            if(isspace(fmt))
                while(isspace(chr))
                    if(!scanner->scan(scanner->where, &chr))
                        return result;
            else if(fmt != chr)
                return result;
            continue;
        }

        format_data_t format_data;
        format_data.charset = NULL;

        format_data.ignore = false;
        if(*format == '*')
            format_data.ignore = true, format++;

        format_data.width = 0;
        while(isdigit(*format))
            format_data.width = format_data.width * 10 + (*format++ - '0');
        
        format_data.length = LEN_NONE;
        if(*format == 'h' && *(format + 1) == 'h')
            format_data.length = LEN_hh, format += 2;
        else if(*format == 'h')
            format_data.length = LEN_h, format++;
        else if(*format == 'l' && *(format + 1) == 'l')
            format_data.length = LEN_ll, format += 2;
        else if(*format == 'l')
            format_data.length = LEN_l, format++;
        else if(*format == 'j')
            format_data.length = LEN_j, format++;
        else if(*format == 'z')
            format_data.length = LEN_z, format++;
        else if(*format == 't')
            format_data.length = LEN_t, format++;

        if(*format == 'c')
            format_data.specifier = SPEC_c, format++;
        else if(*format == 's')
            format_data.specifier = SPEC_s, format++;
        else if(*format == 'd')
            format_data.specifier = SPEC_d, format++;
        else if(*format == 'i')
            format_data.specifier = SPEC_i, format++;
        else if(*format == 'u')
            format_data.specifier = SPEC_u, format++;
        else if(*format == 'o')
            format_data.specifier = SPEC_o, format++;
        else if(*format == 'x')
            format_data.specifier = SPEC_x, format++;
        else if(*format == 'p')
            format_data.specifier = SPEC_p, format++;
        else if(*format == 'n')
            format_data.specifier = SPEC_n, format++;
        else if(*format == '[')
        {
            format++;
            if(*format == '^')
                format_data.specifier = SPEC_NSET, format++;
            else
                format_data.specifier = SPEC_SET;

            // TODO: read set and match closing bracket
        }

        eval_format(format_data, args, scanner, &result);
        if(format_data.charset != NULL)
            free(format_data.charset);
    }

    return result;
}

int vsscanf(const char* str, const char* format, va_list arg)
{
    scanner_t scanner;
    scanner.scan = scanner_buffer;
    scanner.where = str;
    return vsscanf_internal(&scanner, format, arg);
}

int sscanf(const char* str, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vsscanf(str, format, args);
    va_end(args);
    return ret;
}

int vfscanf(FILE* stream, const char* format, va_list arg)
{
    scanner_t scanner;
    scanner.scan = scanner_stream;
    scanner.where = stream;
    return vsscanf_internal(&scanner, format, arg);
}

int fscanf(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stream, format, args);
    va_end(args);
    return ret;
}

int vscanf(const char* format, va_list arg)
{
    return vfscanf(stdin, format, arg);
}

int scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vscanf(format, args);
    va_end(args);
    return ret;
}