#pragma once

#include <stdarg.h>
#include <stdint.h>

#define MAX_TTYS  4

typedef struct screenpos
{
    uint16_t x;
    uint16_t y;
} screenpos_t;

#ifdef __cplusplus
extern "C" {
#endif

void tty_init();

uint32_t tty_get_textcolor_fg();
uint32_t tty_get_textcolor_bg();

void tty_set_textcolor(uint32_t fg, uint32_t bg);
void tty_set_textcolor_fg(uint32_t fg);
void tty_set_textcolor_bg(uint32_t bg);

screenpos_t tty_getpos();
void tty_setpos(screenpos_t pos);

void tty_clear();

void tty_print(const char* str);


#ifdef __cplusplus
}
#endif