#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

static long long strtonum(const char* str, char** endptr, int base, long long upper_bound, long long lower_bound)
{
    bool isNegative = false;
    if(*str == '-')
        isNegative = true, str++;
    else if(*str == '+')
        str++;

    if((base == 0 || base == 16) && (*str == '0' && tolower(*(str + 1)) == 'x'))
    {
        str += 2;
        base = 16;
    }

    if(base == 0)
        if((*str == '0'))
            base = 8;
        else
            base = 10;

    long long sum = 0;
    while(isalnum(*str))
    {
        char c = tolower(*str++);
        if(c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            c -= '0';

        if(c >= base)
            break;

        long long new_val = (sum * base) + c;

        if(new_val <= sum || (isNegative && -new_val <= lower_bound) || (!isNegative && new_val >= upper_bound))
            return isNegative ? lower_bound : upper_bound;

        sum = new_val;
    }

    if(endptr)
        *endptr = (char*)str;

    return isNegative ? -sum : sum;
}

static unsigned long long strtounum(const char* str, char** endptr, int base, unsigned long long upper_bound)
{
    if(*str == '+')
        str++;

    if(base > 36)
        base = 10;

    if((base == 0 || base == 16) && (*str == '0' && tolower(*(str + 1)) == 'x'))
    {
        str += 2;
        base = 16;
    }

    if(base == 0)
        if((*str == '0'))
            base = 8;
        else
            base = 10;

    unsigned long long sum = 0;
    while(isalnum(*str))
    {
        char c = tolower(*str++);
        if(c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            c -= '0';

        if(c >= base)
            break;

        unsigned long long new_val = (sum * base) + c;

        if(new_val <= sum || (new_val >= upper_bound))
            return upper_bound;

        sum = new_val;
    }

    if(endptr)
        *endptr = (char*)str;

    return sum;
}

int atoi(const char* str)
{
    return (int)strtonum(str, NULL, 10, INT32_MAX, INT32_MIN);
}

long atol(const char* str)
{
    return (long)strtonum(str, NULL, 10, INT64_MAX, INT64_MAX);
}

long long atoll(const char* str)
{
    return strtonum(str, NULL, 10, INT64_MAX, INT64_MIN);
}

unsigned int atoui(const char* str)
{
    return (unsigned int)strtounum(str, NULL, 10, UINT32_MAX);
}

unsigned long atoul(const char* str)
{
    return (unsigned long)strtounum(str, NULL, 10, UINT64_MAX);
}

unsigned long long atoull(const char* str)
{
    return strtounum(str, NULL, 10, UINT64_MAX);
}

int strtoi(const char* str, char** endptr, int base)
{
    return (int)strtonum(str, endptr, base, INT32_MAX, INT32_MIN);
}

long strtol(const char* str, char** endptr, int base)
{
    return (long)strtonum(str, endptr, base, INT64_MAX, INT64_MAX);
}

long long strtoll(const char* str, char** endptr, int base)
{
    return strtonum(str, endptr, base, INT64_MAX, INT64_MIN);
}

unsigned int strtoui(const char* str, char** endptr, int base)
{
    return (unsigned int)strtounum(str, endptr, base, UINT32_MAX);
}

unsigned long strtoul(const char* str, char** endptr, int base)
{
    return (unsigned long)strtounum(str, endptr, base, UINT64_MAX);
}

unsigned long long strtoull(const char* str, char** endptr, int base)
{
    return strtounum(str, endptr, base, UINT64_MAX);
}

static char* numtostr(long long value, char* str, int base)
{
    if(value == 0)
    {
        *str++ = '0';
        *str = '\0';
        return str;
    }

    bool isNegative = value < 0;

    size_t i = 0;
    while(value != 0)
    {
        long long rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';  
        value /= base;
    }

    if(isNegative)
        str[i++] = '-';

    str[i] = '\0';

    return strrev(str);
}

char* unumtostr(unsigned long long value, char* str, int base)
{
    if(value == 0)
    {
        *str++ = '0';
        *str = '\0';
        return str;
    }

    size_t i = 0;
    while(value != 0)
    {
        unsigned long long rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';  
        value /= base;
    }

    str[i] = '\0';

    return strrev(str);
}

char* itoa(int value, char* str, int base)
{
    return numtostr(value, str, base);
}

char* ltoa(long value, char* str, int base)
{
    return numtostr(value, str, base);
}

char* lltoa(long long value, char* str, int base)
{
    return numtostr(value, str, base);
}

char* uitoa(unsigned int value, char* str, int base)
{
    return unumtostr(value, str, base);
}

char* ultoa(unsigned long value, char* str, int base)
{
    return unumtostr(value, str, base);
}

char* ulltoa(unsigned long long value, char* str, int base)
{
    return unumtostr(value, str, base);
}
