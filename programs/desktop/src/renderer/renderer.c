#include "renderer.h"
#include <syscalls.h>

#define SCREEN_BPP 32

typedef struct display_info
{
    void* framebuffer;
    size_t width;
    size_t height;
    uint64_t pitch;
} display_info_t;

static display_info_t display_info;

void renderer_init(void* framebuffer, size_t width, size_t height, uint64_t pitch)
{
    display_info.width = width;
    display_info.height = height;
    display_info.pitch = pitch;
    display_info.framebuffer = map_mem(framebuffer, height * pitch);

    renderer_clear(0x00000000);
}

void renderer_clear(uint32_t color)
{
    for(int y = 0; y < display_info.height; y++)
        for(int x = 0; x < display_info.width; x++)
            renderer_put_pixel(x, y, color);
}

void renderer_put_pixel(uint64_t x, uint64_t y, uint32_t color)
{
    uint32_t pixel_offset = y * display_info.pitch + (x * (SCREEN_BPP / 8));
    *(uint32_t*)(pixel_offset + display_info.framebuffer) = color;
}