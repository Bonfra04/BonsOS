#include "renderer.h"
#include <syscalls.h>
#include <string.h>
#include <stdlib.h>

#define SCREEN_BPP 32

typedef struct display_info
{
    void* framebuffer;
    size_t width;
    size_t height;
    uint64_t pitch;
} display_info_t;

static display_info_t display_info;
void* backbuffer;

void renderer_init(void* framebuffer, size_t width, size_t height, uint64_t pitch)
{
    display_info.width = width;
    display_info.height = height;
    display_info.pitch = pitch;
    display_info.framebuffer = map_mem(framebuffer, height * pitch);
    backbuffer = map_mem(0, height * pitch);

    renderer_clear(0x00000000);
    swap_buffers();
}

inline uint64_t display_width()
{
    return display_info.width;
}

inline uint64_t display_height()
{
    return display_info.height;
}

void renderer_clear(uint32_t color)
{
    for(int y = 0; y < display_info.height; y++)
        for(int x = 0; x < display_info.width; x++)
            renderer_put_pixel(x, y, color);
}

void renderer_put_pixel(uint64_t x, uint64_t y, uint32_t color)
{
    if(x >= display_info.width || y >= display_info.height)
        return;

    uint32_t pixel_offset = y * display_info.pitch + (x * (SCREEN_BPP / 8));
    *(uint32_t*)(pixel_offset + backbuffer) = color;
}

void fill_rect(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t color)
{
    for(int _y = y; _y < y + height; _y++)
        for(int _x = x; _x < x + width; _x++)
            renderer_put_pixel(_x, _y, color);
}

inline void swap_buffers()
{
    memcpy(display_info.framebuffer, backbuffer, display_info.height * display_info.pitch);
}