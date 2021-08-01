#include "stdlib.h"
#include "stdbool.h"
#include "string.h"
#include "ctype.h"
#include "limits.h"

static char* skipwhite(char* q)
{
    char* p = q;
    while (isspace(*p))
        ++p;
    return p;
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