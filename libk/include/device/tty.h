#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <device/vga.h>

#define MAX_TTYS  4

typedef struct screenpos
{
    uint8_t x;
    uint8_t y;
} screenpos_t;

#ifdef __cplusplus
extern "C" {
#endif

void tty_init();
void tty_activate(int id);

// Explicit routines

textcolor_t tty_id_get_textcolor_fg(int id);
textcolor_t tty_id_get_textcolor_bg(int id);

void tty_id_set_textcolor(int id, textcolor_t fg, textcolor_t bg);
void tty_id_set_textcolor_fg(int id, textcolor_t fg);
void tty_id_set_textcolor_bg(int id, textcolor_t bg);

screenpos_t tty_id_getpos(int id);
void tty_id_setpos(int id, screenpos_t pos);

void tty_id_clear(int id);

void tty_id_print(int id, const char* str);

int tty_id_printf(int id, const char *format, ...);
int tty_id_scanf(int id, const char* format, ...);

// Implicit routines

textcolor_t tty_get_textcolor_fg();
textcolor_t tty_get_textcolor_bg();

void tty_set_textcolor(textcolor_t fg, textcolor_t bg);
void tty_set_textcolor_fg(textcolor_t fg);
void tty_set_textcolor_bg(textcolor_t bg);

screenpos_t tty_getpos();
void tty_setpos(screenpos_t pos);

void tty_clear();

void tty_print(const char* str);

int tty_printf(const char *format, ...);
int tty_scanf(const char* format, ...);


#ifdef __cplusplus
}
#endif