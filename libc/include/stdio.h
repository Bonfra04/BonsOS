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

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef size_t fpos_t;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int fclose(FILE* stream);
FILE* fopen(const char* filename, const char* mode);
FILE* freopen(const char* filename, const char* mode, FILE* stream);
int remove(const char* filename);

int fgetpos(FILE* stream, fpos_t* pos);
int fsetpos(FILE* stream, const fpos_t* pos);
int fseek(FILE* stream, long offset, int origin);
long ftell(FILE* stream);
void rewind(FILE* stream);

size_t fread(void* ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE *stream);

int fputc(int character, FILE* stream);
#define putc(character, stream) fputc(character, stream)
int putchar(int character);
int fputs(const char* str, FILE* stream);
int puts(const char* str);

int fgetc(FILE* stream);
#define getc(stream) fgetc(stream)
int getchar(void);
char* fgets(char* str, int num, FILE* stream);
char* gets(char* str);

int feof(FILE* stream);
int ferror(FILE* stream);
void clearerr(FILE* stream);

int vsprintf(char* buf, const char* format, va_list args);
int sprintf(char* buf, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list args);
int snprintf(char* buf, size_t n, const char* format, ...);

int vfprintf(FILE* stream, const char* format, va_list arg);
int fprintf(FILE* stream, const char* format, ...);
int vprintf(const char * format, va_list arg);
int printf(const char* format, ...);

int vsscanf(const char* str, const char* format, va_list arg);
int sscanf(const char* str, const char* format, ...);

int vfscanf(FILE* stream, const char* format, va_list arg);
int fscanf(FILE* stream, const char* format, ...);
int vscanf(const char* format, va_list arg);
int scanf(const char* format, ...);

#ifdef __cplusplus
}
#endif
