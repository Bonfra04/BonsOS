#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUFSIZ 512

#define FOPEN_MAX 32
#define EOF -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IOFBF 64
#define _IOLBF 128
#define _IONBF 192

typedef size_t fpos_t;

typedef struct _FILE
{
    void* address;
    unsigned char flags;
    void* buffer;
    size_t buffer_size;
    size_t buffered;
} FILE;

int remove(const char* filename);

FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);

void setbuf(FILE* stream, char* buffer);

int sprintf(char* buf, const char* format, ...);
int vsprintf(char* buf, const char* format, va_list args);
int snprintf(char* buf, size_t n, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list arg);
int sscanf(const char* s, const char* format, ...);
int vsscanf(const char* s, const char* format, va_list args);

size_t fread(void* ptr, size_t size, size_t count, FILE* stream);

int fgetpos(FILE* stream, fpos_t* pos);
int fseek(FILE* stream, long int offset, int origin);
int fsetpos(FILE* stream, const fpos_t* pos);
long int ftell(FILE* stream);
void rewind(FILE* stream);

int feof(FILE* stream);

#ifdef __cplusplus
}
#endif