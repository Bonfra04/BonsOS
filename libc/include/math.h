#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char cmax(char a, char b);
char cmin(char a, char b);
short smax(short a, short b);
short smin(short a, short b);
int imax(int a, int b);
int imin(int a, int b);
long lmax(long a, long b);
long lmin(long a, long b);
long long llmax(long long a, long long b);
long long llmin(long long a, long long b);

unsigned char ucmax(unsigned char a, unsigned char b);
unsigned char ucmin(unsigned char a, unsigned char b);
unsigned short usmax(unsigned short a, unsigned short b);
unsigned short usmin(unsigned short a, unsigned short b);
unsigned int uimax(unsigned int a, unsigned int b);
unsigned int uimin(unsigned int a, unsigned int b);
unsigned long ulmax(unsigned long a, unsigned long b);
unsigned long ulmin(unsigned long a, unsigned long b);
unsigned long long ullmax(unsigned long long a, unsigned long long b);
unsigned long long ullmin(unsigned long long a, unsigned long long b);

#ifdef __cplusplus
}
#endif
