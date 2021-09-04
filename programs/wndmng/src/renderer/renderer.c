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
void* backbuffer[2];
uint8_t current_buffer;

void renderer_init(void* framebuffer, size_t width, size_t height, uint64_t pitch)
{
    display_info.width = width;
    display_info.height = height;
    display_info.pitch = pitch;
    display_info.framebuffer = map_mem(framebuffer, height * pitch);
    backbuffer[0] = map_mem(0, height * pitch);
    backbuffer[1] = map_mem(0, height * pitch);
    memset(backbuffer[1], 0xFF, height * pitch);
    current_buffer = 0;

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
    *(uint32_t*)(pixel_offset + backbuffer[current_buffer]) = color;
}

void fill_rect(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t color)
{
    for(int _y = y; _y < y + height; _y++)
        for(int _x = x; _x < x + width; _x++)
            renderer_put_pixel(_x, _y, color);
}

void draw_image(tga_t* image, uint64_t _x, uint64_t _y)
{
    for(int y = 0; y < image->height; y++)
        for(int x = 0; x < image->width; x++)
        {
            uint32_t rgba = image->data[x + y * image->width];
            if(rgba & 0xFF000000)
                renderer_put_pixel(_x + x, _y + y, rgba);
        }
}

void swap_buffers()
{
    uint64_t* a = (uint64_t*)backbuffer[current_buffer];
    uint64_t* b = (uint64_t*)backbuffer[1 - current_buffer];
    uint64_t* c = (uint64_t*)(display_info.framebuffer);
    for(uint64_t i = 0; i < display_info.height * display_info.pitch / sizeof(uint64_t); i++)
        if(a[i] != b[i])
            c[i] = a[i];
    current_buffer = 1 - current_buffer;    
}