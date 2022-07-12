#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

void* memcpy(void* destination, const void* source, size_t num)
{
    char* dst = destination;
    const char* src = source;
    while(num--)
        *dst++ = *src++;
    return destination;
}

void* memmove(void* destination, const void* source, size_t num)
{
    unsigned char* dst =destination;
    const unsigned char* src = source;
    if (src < dst)
        for (src += num, dst += num; num--;)
            *--dst = *--src;
    else
        while (num--)
            *dst++ = *src++;
    return destination;
}

char* strcpy(char* destination, const char* source)
{
    char* ptr = destination;
    while(*destination++ = *source++);
    return ptr;
}

char* strncpy(char* destination, const char* source, size_t num)
{
    char* ptr = destination;
    do
    {
        if(!num--)
            return ptr;
    } while(*destination++ = *source++);
    while(num--)
        *destination++ = 0;
    return ptr;
}

char* strcat(char* destination, const char* source)
{
    char* ptr = destination;
    while(*destination)
        destination++;
    while(*destination++ = *source++);
    return ptr;
}

char* strncat(char* destination, const char* source, size_t num)
{
    char* ptr = destination;
    while (*destination)
        destination++;
    while(num--)
        if (!(*destination++ = *source++))
            return destination;
    *destination = 0;
    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    const unsigned char* a = ptr1;
    const unsigned char* b = ptr2;
    while(num--)
        if(*a != *b)
            return *a - *b;
        else
            a++, b++;
    return 0;
}

int strcmp(const char* str1, const char* str2)
{
    while(*str1 && *str1 == *str2)
        str1++, str2++;
    if(!*str1)
        return 0;
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
    while(num--)
        if(*str1++ != *str2++)
            return *(unsigned char*)(str1 - 1) - *(unsigned char*)(str2 - 1);
    return 0;
}

const void* memchr(const void* ptr, int value, size_t num)
{
    const unsigned char* p = ptr;
    while(num--)
        if(*p != (unsigned char)value)
            p++;
        else
            return p;
    return 0;
}

const char* strchr(const char* str, int character)
{
    const char* p = str;
    char c;
    while(c = *p++)
        if(c == character)
            return --p;
    return 0;
}

size_t strcspn(const char* str1, const char* str2)
{
    size_t res = 0;
    for(; *str1; str1++, res++)
        for(const char* p = str2; *p; p++)
            if(*p == *str1)
                return res;
    return res;
}

const char* strpbrk(const char* str1, const char* str2)
{
    for(; *str1; str1++)
        for(const char* p = str2; *p; p++)
            if(*p == *str1)
                return str1;
    return NULL;
}

const char* strrchr(const char* str, int character)
{
    const char *sc;

    for (sc = NULL; ; ++str)
    {
        if (*str == character)
            sc = str;
        if (*str == '\0')
            return (char*)sc;
    }
}

size_t strspn(const char* str1, const char* str2)
{
    const char *sc1;
    for (sc1 = str1; *sc1; ++sc1)
        for (const char* sc2 = str2; ; ++sc2)
            if (!*sc2)
                return sc1 - str1;
            else if (*sc1 == *sc2)
                break;
    return sc1 - str1;
}

const char* strstr(const char* str1, const char* str2)
{
    if(!*str2)
        return str1;

    for(; *str1; str1++)
        for(size_t i = 0; *str2 && str1[i]; i++)
            if(str1[i] != str2[i])
                break;
            else if (!str2[i + 1])
                return str1;

    return NULL;
}

char* strtok (char* string, const char* delim) {
    static char * index;

    if (string != NULL)
        index = string;
    else
        string = index;

    if (*index == '\0')
        return NULL;

    while (*index != '\0') {
        for (int i = 0; delim[i] != '\0'; i++)
        {
            if (*index == delim[i])
            {
                if (index == string)
                {
                    index++;
                    string++;
                }
                else
                {
                    *index = '\0';
                    break;
                }
            }
        }

        if (*index == '\0')
        {
            index++;
            return string;
        }

        index++;
    }

    return string;
}

void* memset(void* bufptr, int value, size_t size)
{
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++)
        buf[i] = (unsigned char) value;
    return bufptr;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char* strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

char* strtoupper(char* str)
{
    char* p = str;
    for(; *p; p++)
        *p = toupper(*p);
    return str;
}

char* strdup(const char* str)
{
    size_t len = strlen(str);
    char* ptr = malloc(len + 1);
    if(!ptr)
        return NULL;
    memcpy(ptr, str, len + 1);
    return ptr;
}
