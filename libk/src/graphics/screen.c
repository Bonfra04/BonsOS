#include <graphics/screen.h>
#include <stdint.h>
#include <memory/paging.h>
#include <memory/page_frame_allocator.h>

#define SCREEN_BPP 32

static size_t screen_width;
static size_t screen_height;
static size_t screen_pitch;
static void* framebuffer;

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

void screen_init(size_t width, size_t height, size_t pitch, void* _framebuffer)
{
    screen_width = width;
    screen_height = height;
    screen_pitch = pitch;
    framebuffer = _framebuffer;

    size_t fb_size = height * pitch;
    // identity map fb
    paging_map_global(_framebuffer, _framebuffer, fb_size, PAGE_PRIVILEGE_USER);
    // Deninit frame buffer region
    pfa_deinit_region((uint64_t)_framebuffer, fb_size);

    screen_clear(0xFF000000);
}

void screen_clear(int color)
{
    for(int y = 0; y < screen_height; y++)
        for(int x = 0; x < screen_width; x++)
            screen_plot_pixel(x, y, color);
}

void screen_plot_pixel(size_t x, size_t y, int color)
{
    if(x >= screen_width || y >= screen_height)
        return;

    uint32_t pixel_offset = y * screen_pitch + (x * (SCREEN_BPP / 8));
    *(uint32_t*)(pixel_offset + framebuffer) = color;
}