#include <io/tty.h>
#include <graphics/screen.h>
#include <graphics/text_renderer.h>

#include <containers/queue.h>
#include <containers/deque.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef enum escape_type
{
    ESC_SS2 = 0x8E, ESC_SS3 = 0x8F, ESC_DCS = 0x90, ESC_CSI = 0x9B, ESC_ST = 0x9C,
    ESC_OSC = 0x9D, ESC_SOS = 0x98, ESC_PM = 0x9E, ESC_APC = 0x9F,
    ESC_NONE = 0
} escape_type_t;

typedef struct escape
{
    escape_type_t type;
    union
    {
        uint8_t data[17];
        struct
        {
            uint8_t type;
            uint64_t param0;
            uint64_t param1;
        } csi;
    };
} escape_t;

static uint32_t fg_color;
static uint32_t bg_color;
static tty_pos_t pos;

static size_t row_length;
static size_t col_length;
static size_t char_width;
static size_t char_height;

static char* screen;

static queue_t key_packet;
static deque_t line_queue;

static bool cursor_state;

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

    screen = malloc(row_length * col_length);
    memset(screen, ' ', row_length * col_length);

    line_queue = deque();
    key_packet = queue();

    cursor_state = false;

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

typedef struct escape_params
{
    escape_t escape;
    char seq[20 * 2 + 3];
    size_t len;
} escape_params_t;


#define escape_params_init() (escape_params_t){.escape = {.type = ESC_NONE}, .seq = {0}, .len = 0}

static bool eval_escape(char ch, void(*then)(escape_t), escape_params_t* params)
{
    if(params->escape.type == ESC_NONE)
    {
        switch (ch)
        {
        case 'N': params->escape.type = ESC_SS2; break;
        case 'O': params->escape.type = ESC_SS3; break;
        case 'P': params->escape.type = ESC_DCS; break;
        case '[': params->escape.type = ESC_CSI; break;
        case '\\': params->escape.type = ESC_ST; break;
        case ']': params->escape.type = ESC_OSC; break;
        case 'X': params->escape.type = ESC_SOS; break;
        case '^': params->escape.type = ESC_PM; break;
        case '_': params->escape.type = ESC_APC; break;
        default: return false;
        }
        return true;
    }

    if(params->escape.type == ESC_CSI)
    {
        switch (ch)
        {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'J': case 'K': case 'S': case 'T': case 'm':
            params->seq[params->len++] = '\0';
            params->escape.csi.type = ch;
            params->escape.csi.param0 = isdigit(params->seq[0]) ? strtoi(params->seq, NULL, 10) : 1;
            break;

        case 'H': case 'f':
        {
            params->seq[params->len++] = '\0';
            params->escape.csi.type = ch;
            char* next;
            params->escape.csi.param0 = strtoi(params->seq, &next, 10);
            params->escape.csi.param1 = strtoi(next + 1, NULL, 10);
            break;
        }

        case 'i': case 'n':
        {
            params->seq[params->len++] = '\0';
            params->escape.csi.type = ch;
            params->escape.csi.param0 = isdigit(params->seq[0]) ? params->seq[0] - '0' : 0;
            break;
        }
        
        default:
            params->seq[params->len++] = ch;
            return true;
        }
    }
    else // TODO: other escape types
        return false;

    then(params->escape);
    params->escape.type = ESC_NONE;
    params->len = 0;
    return false;
}

#include "tty_print.h"
#include "tty_read.h"
