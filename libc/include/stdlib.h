#pragma once

#include <stddef.h>

#define RAND_MAX 0xffffffff

typedef struct
{
    int quot;
    int rem;
} div_t;

typedef struct
{
    long quot;
    long rem;
} ldiv_t;

typedef struct
{
    long long quot;
    long long rem;
} lldiv_t;

#ifdef __cplusplus
extern "C" {
#endif

double atof(const char* str);
int atoi(const char* str);
int atoui(const char* str);
long atol(const char* str);
long long atoll(const char* str);
double strtod(const char* str, char** endptr);
float strtof(const char* str, char** endptr);
long strtol(const char* str, char** endptr, int base);
long double strtold(const char* str, char** endptr);
long long strtoll(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);

char* itoa(int value, char* str, int base);
char* uitoa(unsigned int value, char* str, int base);
char* ltoa(long value, char* str, int base);
char* ultoa(unsigned long value, char* str, int base);
char* lltoa(long long value, char* str, int base);
char* ulltoa(unsigned long long value, char* str, int base);

char* ftoa(float value, char* str, int precision);

int rand(void);
void srand(unsigned int seed);

void* malloc(size_t size);
void free(void* ptr);

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*));

int abs(int n);
div_t div(int numer, int denom);
long labs (long n);
ldiv_t ldiv(long numer, long denom);
long long llabs(long long n);
lldiv_t lldiv(long long numer, long long denom);

#ifdef __cplusplus
}
#endif