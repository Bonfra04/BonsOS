#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

static char* skipwhite(char* q)
{
    char* p = q;
    while (isspace(*p))
        ++p;
    return p;
}

double atof(const char* str)
{
    return strtod(str, 0);
}

int atoi(const char* str)
{
    bool isNegative = false;

    int sum = 0;
    if(*str == '-')
        isNegative = true, str++;
    else if(*str == '+')
        str++;

    while(isdigit(*str))
        sum = (sum * 10) + (*str++ - '0');
    return isNegative ? -sum : sum;
}


int atoui(const char* str)
{
    int sum = 0;
    if(*str == '+')
        str++;

    while(isdigit(*str))
        sum = (sum * 10) + (*str++ - '0');
    return sum;
}

long atol(const char* str)
{
    bool isNegative = false;

    long sum = 0;
    if(*str == '-')
        isNegative = true, str++;
    else if(*str == '+')
        str++;

    while(isdigit(*str))
        sum = (sum * 10L) + (*str++ - '0');
    return isNegative ? -sum : sum;
}

long long atoll(const char* str)
{
    bool isNegative = false;

    long long sum = 0;
    if(*str == '-')
        isNegative = true, str++;
    else if(*str == '+')
        str++;

    while(isdigit(*str))
        sum = (sum * 10LL) + (*str++ - '0');
    return isNegative ? -sum : sum;
}

double strtod(const char* str, char** endptr)
{
    double d = 0.0;
    int sign;
    int n = 0;
    const char* p, * a;

    a = p = str;
    p = skipwhite((char*)p);

    sign = 1;
    if (*p == '-')
    {
        sign = -1;
        ++p;
    }
    else if (*p == '+')
        ++p;
    if (isdigit(*p))
    {
        d = (double)(*p++ - '0');
        while (*p && isdigit(*p))
        {
            d = d * 10.0 + (double)(*p - '0');
            ++p;
            ++n;
        }
        a = p;
    }
    else if (*p != '.')
        goto done;
    d *= sign;

    if (*p == '.')
    {
        double f = 0.0;
        double base = 0.1;
        ++p;

        if (isdigit(*p))
        {
            while (*p && isdigit(*p))
            {
                f += base * (*p - '0');
                base /= 10.0;
                ++p;
                ++n;
            }
        }
        d += f * sign;
        a = p;
    }

    if ((*p == 'E') || (*p == 'e'))
    {
        int e = 0;
        ++p;

        sign = 1;
        if (*p == '-')
        {
            sign = -1;
            ++p;
        }
        else if (*p == '+')
            ++p;

        if (isdigit(*p))
        {
            while (*p == '0')
                ++p;
            if (*p == '\0') --p;
            e = (int)(*p++ - '0');
            while (*p && isdigit(*p))
            {
                e = e * 10 + (int)(*p - '0');
                ++p;
            }
            e *= sign;
        }
        else if (!isdigit(*(a - 1)))
        {
            a = str;
            goto done;
        }
        else if (*p == 0)
            goto done;

        if (d == 2.2250738585072011 && e == -308)
        {
            d = 0.0;
            a = p;
            // TODO: set errno
            goto done;
        }
        if (d == 2.2250738585072012 && e <= -308)
        {
            d *= 1.0e-308;
            a = p;
            goto done;
        }
        d *= pow(10.0, (double)e);
        a = p;
    }
    else if (p > str && !isdigit(*(p - 1)))
    {
        a = str;
        goto done;
    }

done:
    if (endptr)
        *endptr = (char*)a;
    return d;
}

float strtof(const char* str, char** endptr)
{
    errno = 0;
    double dresult = strtod(str, endptr);
    float fresult = (float) dresult;

    return fresult;
}

long strtol(const char* str, char** endptr, int base)
{
    long result;
    char* p = skipwhite((char*)str);

    if (*p == '-')
    {
        p++; 
        result = -(strtoul(p, endptr, base));
    } else
    {
        if (*p == '+')
            p += 1;
        result = strtoul(p, endptr, base);
    }
    if ((result == 0) && (endptr != 0) && (*endptr == p))
        *endptr = (char*) str;

    return result;
}

long double strtold(const char* str, char** endptr)
{
    long double d = 0.0;
    int sign;
    int n = 0;

    const char* a = str;
    const char* p = skipwhite((char*)str);

    sign = 1;
    if (*p == '-')
    {
        sign = -1;
        ++p;
    }
    else if (*p == '+')
        ++p;
    if (isdigit(*p))
    {
        d = (long double)(*p++ - '0');
        while (*p && isdigit(*p))
        {
            d = d * 10.0L + (long double)(*p - '0');
            ++p;
            ++n;
        }
        a = p;
    }
    else if (*p != '.')
        goto done;
    d *= sign;

    if (*p == '.')
    {
        long double f = 0.0;
        long double base = 0.1;
        ++p;

        if (isdigit(*p))
        {
            while (*p && isdigit(*p))
            {
                f += base * (*p - '0');
                base /= 10.0;
                ++p;
                ++n;
            }
        }
        d += f * sign;
        a = p;
    }

    if ((*p == 'E') || (*p == 'e'))
    {
        int e = 0;
        ++p;

        sign = 1;
        if (*p == '-')
        {
            sign = -1;
            ++p;
        }
        else if (*p == '+')
            ++p;

        if (isdigit(*p))
        {
            while (*p == '0')
                ++p;
            if (*p == '\0') --p;
            e = (int)(*p++ - '0');
            while (*p && isdigit(*p))
            {
                e = e * 10 + (int)(*p - '0');
                ++p;
            }
            e *= sign;
        }
        else if (!isdigit(*(a - 1)))
        {
            a = str;
            goto done;
        }
        else if (*p == 0)
            goto done;

        if (d == 2.2250738585072011 && e == -308)
        {
            d = 0.0;
            a = p;
            // TODO: set errno
            goto done;
        }
        if (d == 2.2250738585072012 && e <= -308)
        {
            d *= 1.0e-308;
            a = p;
            goto done;
        }
        d *= pow(10.0, (long double)e);
        a = p;
    }
    else if (p > str && !isdigit(*(p - 1)))
    {
        a = str;
        goto done;
    }

done:
    if (endptr)
        *endptr = (char*)a;
    return d;
}

long long strtoll(const char* str, char** endptr, int base)
{
    long long result;
    char *p = skipwhite((char*)str);

    if (*p == '-')
    {
        p++; 
        result = -(strtoull(p, endptr, base));
    } else
    {
        if (*p == '+')
            p += 1;
        result = strtoull(p, endptr, base);
    }
    if ((result == 0) && (endptr != 0) && (*endptr == p))
        *endptr = (char*) str;

    return result;
}

unsigned long strtoul(const char* str, char** endptr, int base)
{
    const char* s = skipwhite((char*)str);
    int c = *s++;

    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    unsigned long cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    int cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;

    int any;
    unsigned long acc;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }

    if (any < 0) {
        acc = ULONG_MAX;
        //TODO: errno = ERANGE;
    }

    if (endptr != 0)
        *endptr = (char*)(any ? s - 1 : str);

    return acc;
}

unsigned long long strtoull(const char* str, char** endptr, int base)
{
    const char* s = skipwhite((char*)str);
    int c = *s++;

    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    unsigned long long cutoff = (unsigned long long)ULLONG_MAX / (unsigned long long)base;
    int cutlim = (unsigned long long)ULLONG_MAX % (unsigned long long)base;

    int any;
    unsigned long long acc;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }

    if (any < 0) {
        acc = ULLONG_MAX;
        //TODO: errno = ERANGE;
    }

    if (endptr != 0)
        *endptr = (char*)(any ? s - 1 : str);

    return acc;
}

char* itoa(int value, char* str, int base)
{
    int i = 0; 
    bool isNegative = false; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    if (value < 0 && base == 10) 
    { 
        isNegative = true; 
        value = -value; 
    } 

    while (value != 0) 
    { 
        int rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    } 
  
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* uitoa(unsigned int value, char* str, int base)
{
    int i = 0; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    while (value != 0) 
    { 
        unsigned int rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    }
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* ltoa(long value, char* str, int base)
{
    int i = 0; 
    bool isNegative = false; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    if (value < 0 && base == 10) 
    { 
        isNegative = true; 
        value = -value; 
    } 

    while (value != 0) 
    { 
        long rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    } 
  
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* ultoa(unsigned long value, char* str, int base)
{
    int i = 0; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    while (value != 0) 
    { 
        unsigned long rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    }
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* lltoa(long long value, char* str, int base)
{
    int i = 0; 
    bool isNegative = false; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    if (value < 0 && base == 10) 
    { 
        isNegative = true; 
        value = -value; 
    } 

    while (value != 0) 
    { 
        long long rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    } 
  
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* ulltoa(unsigned long long value, char* str, int base)
{
    int i = 0; 
  
    if (value == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 

    while (value != 0) 
    { 
        unsigned long long rem = value % base; 
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; 
        value = value / base; 
    }
  
    str[i] = '\0';
  
    strrev(str); 
    return str; 
}

char* ftoa(float value, char* str, int precision)
{
    bool isNegative = value < 0;

    value = value < 0 ? -value : value;

    char* p = str;
    if(isNegative)
        *p++ = '-';
    uitoa((unsigned int)value, p, 10);
    value -= (unsigned int)value;
    while(precision--)
        value *= 10;
    p += strlen(p);
    *p++ = '.';
    itoa((unsigned int)value, p, 10);

    return str;
}
