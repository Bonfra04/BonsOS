#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include <x86/ports.h>
#include <device/keyboard.h>
#include <device/tty.h>
#include <graphics/screen.h>
#include <graphics/renderer.h>

#define TAB_SIZE 4

uint32_t fg_color;
uint32_t bg_color;
screenpos_t pos;

size_t row_length;
size_t col_length;
size_t char_width;
size_t char_height;

void tty_init()
{
    fg_color = 0xFFFFFFFF;
    bg_color = 0x00000000;
    pos.x = 0;
    pos.y = 0;

    char_width = renderer_charwidth();
    char_height = renderer_charheight();
    row_length = screen_get_width() / char_width;
    col_length = screen_get_height() / char_height;

    screen_clear(bg_color);
}

static void tty_printchar(char ch)
{
    bool linefeed = false;

    if (ch == '\n')
    {
        pos.x = 0;
        linefeed = true;
    }
    else if (ch == '\t')
    {
        for(int i = 0; i < TAB_SIZE; i++)
            tty_printchar(' ');
    }
    else if (ch == '\b')
    {
        if(pos.y != 0 || pos.x != 0)
        {
            if (pos.x == 0)
            {
                pos.x = row_length;
                pos.y--;
            }

            pos.x--;
            renderer_putchar(pos.x * char_width, pos.y * char_height, ' ', fg_color, bg_color);
        }
    }
    else
    {
        renderer_putchar(pos.x * char_width, pos.y * char_height, ch, fg_color, bg_color);
        if (++pos.x == row_length)
        {
            pos.x = 0;
            linefeed = true;
        }
    }

    if (linefeed)
    {
        /*
        memcpy(cons->screen + cons->ybuf * VGA_SCREEN_COLS - VGA_SCREEN_SIZE,
               cons->screen + cons->ybuf * VGA_SCREEN_COLS,
               VGA_SCREEN_COLS * sizeof(uint16_t));
        */

        pos.y++;
        
        if (pos.y == col_length)
        {
            pos.y--;

            /*
            memsetw(cons->screen + cons->ybuf * VGA_SCREEN_COLS,
                    cons->textcolor | ' ',
                    VGA_SCREEN_COLS * sizeof(uint16_t));
            */
        }
    }
}

uint32_t tty_get_textcolor_fg()
{
    return fg_color;
}

uint32_t tty_get_textcolor_bg()
{
    return bg_color;
}

void tty_set_textcolor(uint32_t fg, uint32_t bg)
{
    fg_color = fg;
    bg_color = bg;
}

void tty_set_textcolor_fg(uint32_t fg)
{
    fg_color = fg;
}

void tty_set_textcolor_bg(uint32_t bg)
{
    bg_color = bg;
}

screenpos_t tty_getpos()
{
    return pos;
}

void tty_setpos(screenpos_t _pos)
{
    pos = _pos;
}

void tty_clear()
{
    pos.x = 0;
    pos.y = 0;
    screen_clear(bg_color);
}

void tty_print(const char* str)
{
    while(*str)
        tty_printchar(*str++);
}