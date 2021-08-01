#pragma once

int atoi(const char* str);
int atoui(const char* str);
long atol(const char* str);
long strtol(const char* str, char** endptr, int base);
long double strtold(const char* str, char** endptr);
unsigned long strtoul(const char* str, char** endptr, int base);

char* itoa(int value, char* str, int base);
char* uitoa(unsigned int value, char* str, int base);
char* ltoa(long value, char* str, int base);
char* ultoa(unsigned long value, char* str, int base);
char* lltoa(long long value, char* str, int base);
char* ulltoa(unsigned long long value, char* str, int base);
