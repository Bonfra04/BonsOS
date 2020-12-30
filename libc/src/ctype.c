#include <ctype.h>

int isalnum(int c)
{
    return (c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

int isalpha(int c)
{
    return (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

int isblank(int c)
{
    return (c == 0x9) || (c == 0x20);
}

int iscntrl(int c)
{
    return (c >= 0x0 && c <= 0x1F) || (c == 0x7F);
}

int isdigit(int c)
{
    return (c >= 0x30 && c <= 0x39);
}

int isgraph(int c)
{
    return (c >= 0x21 && c <= 0x7E);
}

int islower(int c)
{
    return (c >= 0x61 && c <= 0x7A);
}

int isprint(int c)
{
    return (c >= 0x20 && c <= 0x7E);
}

int ispunct(int c)
{
    return (c >= 0x30 && c <= 0x39) || (c >= 0x3A && c <= 0x40) || (c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7E);
}

int isspace(int c)
{
    return (c >= 0x09 && c <= 0x0D) || (c == 0x20);
}

int isupper(int c)
{
    return (c >= 0x41 && c <= 0x5A);
}

int isxdigit(int c)
{
    return (c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x46) || (c >= 0x61 && c <= 0x66);
}

int tolower(int c)
{
    if(isupper(c))
        return c + 0x20;
    else
        return c;
}

int toupper(int c)
{
    if(islower(c))
        return c - 0x20;
    else
        return c;
}