#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include <x86/ports.h>
#include <device/keyboard.h>
#include <device/tty.h>
#include <device/vga.h>

#define TAB_SIZE 4

// CRTC ports
#define CRTC_PORT_CMD           0x03d4   ///< Command port for CRT controller.
#define CRTC_PORT_DATA          0x03d5   ///< Data port for CRT controller.

// CRTC commands
#define CRTC_CMD_STARTADDR_HI   0x0c   ///< Hi-byte of buffer start address.
#define CRTC_CMD_STARTADDR_LO   0x0d   ///< Lo-byte of buffer start address.
#define CRTC_CMD_CURSORADDR_HI  0x0e   ///< Hi-byte of cursor start address.
#define CRTC_CMD_CURSORADDR_LO  0x0f   ///< Lo-byte of cursor start address.

typedef struct tty_state
{
    uint16_t    textcolor;       ///< Current fg/bg color (shifted).
    uint16_t    textcolor_orig;  ///< Original, non-override text color.
    screenpos_t pos;             ///< Current screen position.
    uint8_t     ybuf;            ///< Virtual buffer y position.
    uint16_t*   screen;          ///< Virtual screen buffer for 50 rows.
    uint16_t*   tlcorner;        ///< Points to char in top-left corner.
} tty_state_t;

static tty_state_t tty[MAX_TTYS];      ///< All virtual consoles.
static tty_state_t* active_tty;        ///< The currently visible console.
static uint8_t active_tty_index;       ///< The currently visible console index.

static void update_buffer_offset()
{
    int offset = (int)(active_tty->tlcorner - (uint16_t *)VGA_SCREEN_BUFFER);

    uint8_t save = inportb(CRTC_PORT_CMD);

    outportb(CRTC_PORT_CMD, CRTC_CMD_STARTADDR_LO);
    outportb(CRTC_PORT_DATA, (uint8_t)offset);
    outportb(CRTC_PORT_CMD, CRTC_CMD_STARTADDR_HI);
    outportb(CRTC_PORT_DATA, (uint8_t)(offset >> 8));

    outportb(CRTC_PORT_CMD, save);
}

static void update_cursor()
{
    int offset = active_tty->ybuf * VGA_SCREEN_COLS + active_tty->pos.x + (int)(active_tty->screen - (uint16_t*)VGA_SCREEN_BUFFER);

    uint8_t save = inportb(CRTC_PORT_CMD);

    outportb(CRTC_PORT_CMD, CRTC_CMD_CURSORADDR_LO);
    outportb(CRTC_PORT_DATA, (uint8_t)offset);
    outportb(CRTC_PORT_CMD, CRTC_CMD_CURSORADDR_HI);
    outportb(CRTC_PORT_DATA, (uint8_t)(offset >> 8));

    outportb(CRTC_PORT_CMD, save);
}

void tty_init()
{
    uint16_t *screenptr = (uint16_t*)VGA_SCREEN_BUFFER;
    for (int id = 0; id < MAX_TTYS; id++) {
        tty[id].textcolor      = color(TEXTCOLOR_WHITE, TEXTCOLOR_BLACK);
        tty[id].textcolor_orig = tty[id].textcolor;
        tty[id].pos.x          = 0;
        tty[id].pos.y          = 0;
        tty[id].ybuf           = 0;
        tty[id].screen         = screenptr;
        tty[id].tlcorner       = screenptr;
        screenptr += 0x1000; // each screen is 4K words.
        tty_id_clear(id);
    }
    active_tty_index = 0;
    active_tty = &tty[active_tty_index];
}

static void tty_printchar(tty_state_t *cons, char ch)
{
    bool linefeed = false;

    if (ch == '\n')
    {
        cons->pos.x = 0;
        linefeed = true;
    }
    else if (ch == '\t')
    {
        for(int i = 0; i < TAB_SIZE; i++)
            tty_printchar(cons, ' ');
    }
    else if (ch == '\b')
    {
        if(cons->pos.y != 0 ||cons->pos.x != 0)
        {
            if (cons->pos.x == 0)
            {
                cons->pos.x = VGA_SCREEN_COLS;
                cons->pos.y--;
                cons->ybuf--;
            }

            int offset = cons->ybuf * VGA_SCREEN_COLS + --cons->pos.x;
            cons->screen[offset] = cons->textcolor | ' ';
        }
    }
    else
    {
        uint16_t value = cons->textcolor | ch;
        int offset = cons->ybuf * VGA_SCREEN_COLS + cons->pos.x;
        cons->screen[offset] = value;
        if (++cons->pos.x == VGA_SCREEN_COLS)
        {
            cons->pos.x = 0;
            linefeed = true;
        }
    }

    if (linefeed)
    {
        memcpy(cons->screen + cons->ybuf * VGA_SCREEN_COLS - VGA_SCREEN_SIZE,
               cons->screen + cons->ybuf * VGA_SCREEN_COLS,
               VGA_SCREEN_COLS * sizeof(uint16_t));

        ++cons->pos.y;
        ++cons->ybuf;

        if (cons->pos.y == VGA_SCREEN_ROWS)
        {
            --cons->pos.y;

            if (cons->ybuf == VGA_SCREEN_ROWS * 2)
                cons->ybuf -= VGA_SCREEN_ROWS;

            memsetw(cons->screen + cons->ybuf * VGA_SCREEN_COLS,
                    cons->textcolor | ' ',
                    VGA_SCREEN_COLS * sizeof(uint16_t));

            cons->tlcorner = cons->screen + (cons->ybuf + 1) * VGA_SCREEN_COLS - VGA_SCREEN_SIZE;
            if (cons == active_tty)
                update_buffer_offset();
        }
    }

    if (cons == active_tty)
        update_cursor();
}

static void tty_print(int id, const char* str)
{
    if ((id < 0) || (id >= MAX_TTYS))
        id = 0;

    tty_state_t *cons = &tty[id];
    for (; *str; ++str)
        tty_printchar(cons, *str);
}

static int tty_va_printf(int id, const char* format, va_list args)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;
    char buffer[8 * 1024];
    int result = vsnprintf(buffer, sizeof(buffer), format, args);

    tty_print(id, buffer);

    return result;
}

static int tty_va_scanf(int id, const char* format, va_list args)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;
    
    tty_state_t* cons = &tty[id];

    char buffer[8 * 1024];
    int ch, p = 0;
    do
    {
        ch = kb_getch();
        if(ch <= 0xFF)
        {
            switch (ch)
            {
            case '\b':
                if(p > 0)
                {
                    p--;
                    tty_printchar(cons, ch);
                }
                break;

            case '\n':
                tty_printchar(cons, ch);
                break;
            
            default:
                buffer[p++] = ch;
                tty_printchar(cons, ch);
            }
        }
        else
            switch (ch - 0xFF)
            {
            case VK_ARROW_LEFT:
                tty_id_setpos(id, (screenpos_t){tty[id].pos.x - 1, tty[id].pos.y});             
                break;

            case VK_ARROW_RIGHT:
                tty_id_setpos(id, (screenpos_t){tty[id].pos.x + 1, tty[id].pos.y});             
                break;
            
            default:
                break;
            }
    } while(ch != '\n' && p < 8 * 1024 - 1);
    buffer[p] = '\0';

    int result = vsscanf(buffer, format, args);

    return result;
}

void tty_activate(int id)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;
    if (&tty[id] == active_tty)
        return;

    active_tty_index = id;
    active_tty = &tty[id];
    update_buffer_offset();
    update_cursor();
}

textcolor_t tty_id_get_textcolor_fg(int id)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    return (textcolor_t)((tty[id].textcolor_orig >> 8) & 0x0f);
}

textcolor_t tty_id_get_textcolor_bg(int id)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    return (textcolor_t)((tty[id].textcolor_orig >> 12) & 0x0f);
}

void tty_id_set_textcolor(int id, textcolor_t fg, textcolor_t bg)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    tty[id].textcolor = tty[id].textcolor_orig = color(fg, bg);
}

void tty_id_set_textcolor_fg(int id, textcolor_t fg)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    tty[id].textcolor = color(fg, tty_id_get_textcolor_bg(id));
    tty[id].textcolor_orig = tty[id].textcolor;
}

void tty_id_set_textcolor_bg(int id, textcolor_t bg)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    tty[id].textcolor = color(tty_id_get_textcolor_fg(id), bg);
    tty[id].textcolor_orig = tty[id].textcolor;
}

screenpos_t tty_id_getpos(int id)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    return tty[id].pos;
}

void tty_id_setpos(int id, screenpos_t pos)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    int diff = (int)pos.y - (int)tty[id].pos.y;
    tty[id].pos = pos;
    tty[id].ybuf = (uint8_t)((int)tty[id].ybuf + diff);
    if (active_tty == &tty[id])
        update_cursor();
}

void tty_id_clear(int id)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    memsetw(tty[id].screen, tty[id].textcolor | ' ', VGA_SCREEN_SIZE * 2);
    tty[id].pos.x    = 0;
    tty[id].pos.y    = 0;
    tty[id].ybuf     = 0;
    tty[id].tlcorner = tty[id].screen;

    if (active_tty == &tty[id])
    {
        update_buffer_offset();
        update_cursor();
    }
}

int tty_id_printf(int id, const char *format, ...)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    va_list args;
    va_start(args, format);
    int result = tty_va_printf(id, format, args);;
    va_end(args);

    return result;
}

int tty_id_scanf(int id, const char* format, ...)
{
    if (id < 0 || id >= MAX_TTYS)
        id = 0;

    va_list args;
    va_start(args, format);
    int result = tty_va_scanf(id, format, args);
    va_end(args);

    return result;
}


// Implicit routines

inline textcolor_t tty_get_textcolor_fg()
{
    return tty_id_get_textcolor_fg(active_tty_index);
}

inline textcolor_t tty_get_textcolor_bg()
{
    return tty_id_get_textcolor_bg(active_tty_index);
}

inline void tty_set_textcolor(textcolor_t fg, textcolor_t bg)
{
    tty_id_set_textcolor(active_tty_index, fg, bg);
}

inline void tty_set_textcolor_fg(textcolor_t fg)
{
    tty_id_set_textcolor_fg(active_tty_index, fg);
}

inline void tty_set_textcolor_bg(textcolor_t bg)
{
    tty_id_set_textcolor_bg(active_tty_index, bg);
}

inline screenpos_t tty_getpos()
{
    return tty_id_getpos(active_tty_index);
}

inline void tty_setpos(screenpos_t pos)
{
    tty_id_setpos(active_tty_index, pos);
}

inline void tty_clear()
{
    tty_id_clear(active_tty_index);
}

int tty_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = tty_va_printf(active_tty_index, format, args);
    va_end(args);
    
    return result;
}

int tty_scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = tty_va_scanf(active_tty_index, format, args);
    va_end(args);

    return result;
}
