#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef long jmp_buf[8];

int setjmp(jmp_buf buffer);
void longjmp(jmp_buf buffer, int value);

#ifdef __cplusplus
}
#endif
