#include <graphics/renderer.h>
#include <stdio.h>
#include <stdint.h>
#include <graphics/screen.h>
#include "default_font.h"

typedef struct psf1_header
{
    uint16_t magic;     /* Magic number */
    uint8_t mode;       /* PSF font mode */
    uint8_t charsize;   /* Character size */
} psf1_header_t;

typedef uint8_t fontchar_t[16];
static fontchar_t bitmaps[256];

void renderer_init()
{
    psf1_header_t* header = (psf1_header_t*)default_font;
    
    size_t advance = sizeof(psf1_header_t);
    for (int ch = 0; ch < 256; ch++)
    {
        for (int i = 0; i < header->charsize; i++)
            bitmaps[ch][i] = default_font[advance + i];
        advance += header->charsize;
    }
}

bool renderer_load_font(const char* filepath)
{
    FILE* f = fopen(filepath, "r");
    if (!f)
        return false;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t content[fsize];
    fread(content, 1, fsize, f);
    fclose(f);

    psf1_header_t* header = (psf1_header_t*)content;
    if (header->magic != 0x0436)
        return false;
    
    size_t advance = sizeof(psf1_header_t);
    for (int ch = 0; ch < 256; ch++)
    {
        for (int i = 0; i < header->charsize; i++)
            bitmaps[ch][i] = content[advance + i];
        advance += header->charsize;
    }

    return true;
}

void renderer_putchar(size_t x, size_t y, char c, uint32_t fg, uint32_t bg)
{
    for(int _y = 0; _y < 16; _y++)
        for(int _x = 0; _x < 8; _x++)
        {
            uint8_t bit = bitmaps[(size_t)c][_y] & (1 << (7 - _x));
            screen_plot_pixel(x + _x, y + _y, bit ? fg : bg);
        }
}

size_t renderer_charwidth()
{
    return 8;
}

size_t renderer_charheight()
{
    return 16;
}