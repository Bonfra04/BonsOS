#include <graphics/text_renderer.h>
#include <graphics/screen.h>
#include <fsys/fsys.h>

#include "default_font.h"

typedef struct psf1_header
{
    uint16_t magic;     /* Magic number */
    uint8_t mode;       /* PSF font mode */
    uint8_t charsize;   /* Character size */
} psf1_header_t;

typedef uint8_t fontchar_t[16];

static fontchar_t bitmaps[256];

static void load_font(psf1_header_t* header)
{
    size_t advance = sizeof(psf1_header_t);
    for (uint16_t ch = 0; ch < 256; ch++)
    {
        for (int i = 0; i < header->charsize; i++)
            bitmaps[ch][i] = ((uint8_t*)header)[advance + i];
        advance += header->charsize;
    }
}

void text_renderer_init()
{
    load_font((psf1_header_t*)default_font);
}

bool text_renderer_load_font(const char* filepath)
{
    file_t f = fsys_open_file(filepath, FSYS_READ);
    if(fsys_error(&f))
        return false;
    if(!fsys_set_position(&f, -1))
        return false;
    uint64_t size = fsys_get_position(&f);
    if(!fsys_set_position(&f, 0))
        return false;

    uint8_t font[size];
    if(fsys_read_file(&f, &font, sizeof(font)) != sizeof(font))
        return false;

    load_font((psf1_header_t*)&font);

    return true;
}

void text_renderer_putchar(size_t x, size_t y, size_t scale, char c, uint32_t fg, uint32_t bg)
{
    for(uint8_t _y = 0; _y < 16 * scale; _y++)
    {
        for(size_t scaling_y = 0; scaling_y < scale; scaling_y++)
            for(uint8_t _x = 0; _x < 8 * scale; _x++)
            {
                uint8_t bit = bitmaps[(size_t)c][_y / scale] & (1 << (7 - _x / scale));
                for(size_t scaling_x = 0; scaling_x < scale; scaling_x++)
                    screen_plot_pixel(x + _x, y + _y, bit ? fg : bg);
            }
    }
}

size_t text_renderer_charwidth(size_t scale)
{
    return 8 * scale;
}

size_t text_renderer_charheight(size_t scale)
{
    return 16 * scale;
}
