#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)

#define _IOFBF 0b01
#define _IOLBF 0b10
#define _IONBF 0b11

typedef struct __FILE
{
    int fd;
    void* buffer;
    size_t buffer_size;
    unsigned char flags;
} FILE;

#define FILE_ERROR  0b0100
#define FILE_EOF    0b1000

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int fclose(FILE* stream);
FILE* fopen(const char* filename, const char* mode);

size_t fread(void* ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE *stream);

int fputc(int character, FILE* stream);
#define putc(character, stream) fputc(character, stream)
int putchar(int character);
int fputs(const char* str, FILE* stream);
int puts(const char* str);

int sprintf(char* buf, const char* format, ...);
int vsprintf(char* buf, const char* format, va_list args);
int snprintf(char* buf, size_t n, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list args);

int feof(FILE* stream);
int ferror(FILE* stream);
void clearerr(FILE* stream);

#ifdef __cplusplus
}
#endif
