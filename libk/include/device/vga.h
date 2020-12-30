#pragma once

#include <stdint.h>

// Visible screen geometry
#define VGA_SCREEN_ROWS     25
#define VGA_SCREEN_COLS     80
#define VGA_SCREEN_SIZE     (VGA_SCREEN_ROWS * VGA_SCREEN_COLS)
#define VGA_SCREEN_BUFFER   0x000b8000

typedef enum
{
    TEXTCOLOR_BLACK     = 0,
    TEXTCOLOR_BLUE      = 1,
    TEXTCOLOR_GREEN     = 2,
    TEXTCOLOR_CYAN      = 3,
    TEXTCOLOR_RED       = 4,
    TEXTCOLOR_MAGENTA   = 5,
    TEXTCOLOR_BROWN     = 6,
    TEXTCOLOR_LTGRAY    = 7,
    TEXTCOLOR_GRAY      = 8,
    TEXTCOLOR_LTBLUE    = 9,
    TEXTCOLOR_LTGREEN   = 10,
    TEXTCOLOR_LTCYAN    = 11,
    TEXTCOLOR_LTRED     = 12,
    TEXTCOLOR_LTMAGENTA = 13,
    TEXTCOLOR_YELLOW    = 14,
    TEXTCOLOR_WHITE     = 15,
} textcolor_t;

static inline uint16_t color(textcolor_t fg, textcolor_t bg)
{
    return (uint16_t)bg << 12 | (uint16_t)fg << 8;
}