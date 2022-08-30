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

uint32_t screen_get_pixel(uint64_t x, uint64_t y)
{
    if(x >= screen_width || y >= screen_height)
        return 0;

    uint32_t pixel_offset = y * screen_pitch + (x * (SCREEN_BPP / 8));
    return *(uint32_t*)(pixel_offset + framebuffer);
}

size_t screen_set_area(uint64_t x, uint64_t y, uint64_t size, const uint32_t* pixels)
{
    if(x >= screen_width || y >= screen_height)
        return 0;

    uint64_t advance = 0;
    uint64_t dy = size / screen_width;
    uint64_t dx = size % screen_width;
    if(y + dy >= screen_height)
        dy = screen_height - y - 1;
    if(x + dx >= screen_width)
        dx = screen_width - y - 1;

    for(uint64_t ypos = y; ypos < y + dy; ypos++)
        for(uint64_t xpos = ypos == y ? x : 0; xpos < screen_width; xpos++)
        {
            uint32_t pixel_offset = ypos * screen_pitch + (xpos * (SCREEN_BPP / 8));
            *(uint32_t*)(pixel_offset + framebuffer) = pixels[advance++];
        }
    for(uint64_t xpos = 0; xpos < dx; xpos++)
    {
        uint32_t pixel_offset = (y + dy) * screen_pitch + (xpos * (SCREEN_BPP / 8));
        *(uint32_t*)(pixel_offset + framebuffer) = pixels[advance++];
    }

    return advance;
}

size_t screen_get_area(uint64_t x, uint64_t y, uint64_t size, uint32_t* pixels)
{
    if(x >= screen_width || y >= screen_height)
        return 0;

    uint64_t advance = 0;
    uint64_t dy = size / screen_width;
    uint64_t dx = size % screen_width;
    if(y + dy >= screen_height)
        dy = screen_height - y - 1;
    if(x + dx >= screen_width)
        dx = screen_width - y - 1;

    for(uint64_t ypos = y; ypos < y + dy; ypos++)
        for(uint64_t xpos = ypos == y ? x : 0; xpos < screen_width; xpos++)
        {
            uint32_t pixel_offset = ypos * screen_pitch + (xpos * (SCREEN_BPP / 8));
            pixels[advance++] = *(uint32_t*)(pixel_offset + framebuffer);
        }
    for(uint64_t xpos = 0; xpos < dx; xpos++)
    {
        uint32_t pixel_offset = (y + dy) * screen_pitch + (xpos * (SCREEN_BPP / 8));
        pixels[advance++] = *(uint32_t*)(pixel_offset + framebuffer);
    }

    return advance;
}