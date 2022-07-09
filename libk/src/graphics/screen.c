#include <graphics/screen.h>
#include <memory/paging.h>
#include <memory/pfa.h>

#define SCREEN_BPP 32

static size_t screen_width;
static size_t screen_height;
static size_t screen_pitch;
static void* framebuffer;

void screen_init(size_t width, size_t height, size_t pitch, void* _framebuffer)
{
    screen_width = width;
    screen_height = height;
    screen_pitch = pitch;
    framebuffer = _framebuffer;

    size_t fb_size = height * pitch;
    paging_map(NULL, framebuffer, framebuffer, fb_size, PAGE_PRIVILEGE_KERNEL);
    pfa_deinit_region(framebuffer, fb_size);

    screen_clear(0x000000);
}

size_t screen_get_width()
{
    return screen_width;
}

size_t screen_get_height()
{
    return screen_height;
}

size_t screen_get_pitch()
{
    return screen_pitch;
}

void screen_clear(uint32_t color)
{
    for(uint64_t y = 0; y < screen_height; y++)
        for(uint64_t x = 0; x < screen_width; x++)
        {
            uint32_t pixel_offset = y * screen_pitch + (x * (SCREEN_BPP / 8));
            *(uint32_t*)(pixel_offset + framebuffer) = color;
        }
}

void screen_plot_pixel(uint64_t x, uint64_t y, uint32_t color)
{
    if(x >= screen_width || y >= screen_height)
        return;

    uint32_t pixel_offset = y * screen_pitch + (x * (SCREEN_BPP / 8));
    *(uint32_t*)(pixel_offset + framebuffer) = color;
}
