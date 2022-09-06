#pragma once

#include <stddef.h>
#include <stdnoreturn.h>

#define RAND_MAX 0xFFFFFFFF

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

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

int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);
unsigned int atoui(const char* str);
unsigned long atoul(const char* str);
unsigned long long atoull(const char* str);

int strtoi(const char* str, char** endptr, int base);
long strtol(const char* str, char** endptr, int base);
long long strtoll(const char* str, char** endptr, int base);
unsigned int strtoui(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);

char* itoa(int value, char* str, int base);
char* ltoa(long value, char* str, int base);
char* lltoa(long long value, char* str, int base);
char* uitoa(unsigned int value, char* str, int base);
char* ultoa(unsigned long value, char* str, int base);
char* ulltoa(unsigned long long value, char* str, int base);

int rand(void);
void srand(unsigned int seed);

void* calloc(size_t num, size_t size);
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

int atexit(void (*func)());
void exit(int status);
int at_quick_exit(void (*func)());
void quick_exit(int status);
noreturn void _Exit(int status);

char* getenv(const char* name);
void setenv(const char* name, const char* val);

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*));

int abs(int n);
long labs(long n);
long long llabs(long long n);
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

#ifdef __cplusplus
}
#endif
