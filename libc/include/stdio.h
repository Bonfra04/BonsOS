#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)

#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3

typedef struct __FILE
{
    int fd;
    unsigned char buffered;
    void* buffer;
    size_t buffer_size;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int fclose(FILE* stream);
FILE* fopen(const char* filename, const char* mode);

size_t fread(void* ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE *stream);

int sprintf(char* buf, const char* format, ...);
int vsprintf(char* buf, const char* format, va_list args);
int snprintf(char* buf, size_t n, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list args);

#ifdef __cplusplus
}
#endif
