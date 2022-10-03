#include <io/tty.h>
#include <graphics/text_renderer.h>
#include <graphics/screen.h>

#include <stdbool.h>

#define TAB_SIZE 4

typedef enum escape_type escape_type_t;
typedef struct escape escape_t;

extern uint32_t fg_color;
extern uint32_t bg_color;
extern tty_pos_t pos;

extern size_t row_length;
extern size_t col_length;
extern size_t char_width;
extern size_t char_height;

extern char* screen;

static bool eval_escape(char ch, void(*then)(escape_t), escape_params_t* params);

void tty_clear()
{
    pos.x = 0;
    pos.y = 0;
    memset(screen, ' ', row_length * col_length);
    screen_clear(bg_color);
}

static void renderchar(char ch)
{
    screen[pos.y * row_length + pos.x] = ch;
    text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ch, fg_color, bg_color);
}

static void renderscreen()
{
    screen_clear(bg_color);
    for(size_t y = 0; y < col_length; y++)
        for(size_t x = 0; x < row_length; x++)
            text_renderer_putchar(x * char_width, y * char_height, 1, screen[y * row_length + x], fg_color, bg_color);
}

static void scroll()
{
    memmove(screen, screen + row_length, col_length * row_length - row_length);
    memset(screen + col_length * row_length - row_length, ' ', row_length);
    renderscreen();
    pos.y--;
}

static void print_escape(escape_t escape)
{
    if(escape.type == ESC_CSI)
    {
        switch (escape.csi.type)
        {
        case 'A': pos.y = pos.y >= escape.csi.param0 ? pos.y - escape.csi.param0 : 0; break;
        case 'B': pos.y = pos.y + escape.csi.param0 >= col_length ? col_length : pos.y + escape.csi.param0; break;
        case 'C': pos.x = pos.x + escape.csi.param0 >= row_length ? row_length : pos.x + escape.csi.param0; break;
        case 'D': pos.x = pos.x >= escape.csi.param0 ? pos.x - escape.csi.param0 : 0; break;
        case 'E': pos.y = pos.y + escape.csi.param0 >= col_length ? col_length : pos.y + escape.csi.param0; pos.x = 0; break;
        case 'F': pos.y = pos.y >= escape.csi.param0 ? pos.y - escape.csi.param0 : 0; pos.x = 0; break;
        case 'G': pos.x = row_length >= escape.csi.param0 ? escape.csi.param0 : row_length; break;
        // TODO: rest of CSI
        }
    }
}

static void print_cntrl(char ch, bool raw)
{
    switch (ch)
    {
    case '\b':
        pos.x = pos.x >= 1 ? pos.x - 1 : 0;
        if(!raw)
            renderchar(' ');
        break;

    case '\t':
        pos.x += TAB_SIZE - pos.x % TAB_SIZE;
        break;

    case '\n':
    case '\f':
        pos.y++;
        if(raw)
            break;
    case '\r':
        pos.x = 0;
        break;
    }
}

static void print_char(char ch, bool raw)
{
    static escape_params_t esc_par = escape_params_init();
    static bool escape = false;

    if(escape)
        escape = eval_escape(ch, print_escape, &esc_par);
    else if(ch == '\033' && !raw)
        escape = true;
    else if(iscntrl(ch))
        print_cntrl(ch, raw);
    else
    {
        renderchar(ch);
        pos.x++;
    }

    if (pos.x == row_length)
    {
        pos.x = 0;
        pos.y++;
    }
    while(pos.y >= col_length)
        scroll();
}

void tty_print(const char* str, bool raw)
{
    while(*str)
        print_char(*str++, raw);
}