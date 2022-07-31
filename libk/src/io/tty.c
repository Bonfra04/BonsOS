#include <io/tty.h>

#include <graphics/screen.h>
#include <graphics/text_renderer.h>
#include <io/keyboard.h>

#include <ctype.h>

#define TAB_SIZE 4

static uint32_t fg_color;
static uint32_t bg_color;
static tty_pos_t pos;

static size_t row_length;
static size_t col_length;
static size_t char_width;
static size_t char_height;

void tty_init()
{
    fg_color = 0xFFFFFFFF;
    bg_color = 0xFF000000;
    pos.x = 0;
    pos.y = 0;

    char_width = text_renderer_charwidth(1);
    char_height = text_renderer_charheight(1);
    row_length = screen_get_width() / char_width;
    col_length = screen_get_height() / char_height;

    tty_clear();
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

tty_pos_t tty_getpos()
{
    return pos;
}

void tty_setpos(tty_pos_t tty_pos)
{
    pos = tty_pos;
}

void tty_clear()
{
    pos.x = 0;
    pos.y = 0;
    screen_clear(bg_color);
}

static void print_char(char ch)
{
    if(ch == '\n')
    {
        pos.x = row_length;
    }
    else if(ch == '\r')
    {
        pos.x = 0;
    }
    else if(ch == '\t')
    {
        if(pos.x % TAB_SIZE == 0)
            pos.x += TAB_SIZE;
        else
            pos.x += TAB_SIZE - pos.x % TAB_SIZE;
    }
    else if(ch == '\b')
    {
        if(pos.x > 0)
        {
            pos.x--;
            text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ' ', fg_color, bg_color);
        }
        else if(pos.y > 0)
        {
            pos.y--;
            pos.x = row_length - 1;
            text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ' ', fg_color, bg_color);
        }
    }
    else
    {
        text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ch, fg_color, bg_color);
        pos.x++;
    }

    if (pos.x == row_length)
    {
        pos.x = 0;
        pos.y++;
    }
}

void tty_print(const char* str)
{
    while(*str)
        print_char(*str++);
}

size_t tty_read(char* buf, size_t size)
{
    size_t read = 0;
    size_t advance = 0;
    while(true)
    {
        keyevent_t k = keyboard_pull();
        if(!k.is_pressed)
            continue;

        switch (k.vt_keycode)
        {
        case '\b':
            if(advance > 0)
            {
                if(--advance < size)
                {
                    *buf = '\0';
                    buf--;
                    read--;
                }
                print_char('\b');
            }
            break;

        case '\n':
            print_char('\n');
            return read;

        default:
            if(isprint(k.vt_keycode) || k.vt_keycode == '\t')
            {
                print_char(k.vt_keycode);
                advance++;
                if(read < size)
                {
                    *buf = k.vt_keycode;
                    buf++;
                    read++;
                }
            }
        }
    }
}
