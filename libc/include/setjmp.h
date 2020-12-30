#pragma once

typedef long jmp_buf[8];
typedef long sigjmp_buf[10];

int setjmp(jmp_buf buffer);
void longjmp(jmp_buf buffer, int value);