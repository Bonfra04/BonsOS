#include <stdlib.h>
#include "renderer/renderer.h"
#include "renderer/tga.h"
#include <syscalls.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "events/event.h"
#include "msg/msg.h"
#include "window/window.h"

void init();
void update();
void render();

static tga_t cursor_tga;

int main(int argc, char* argv[])
{
    init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    run_executable("a:/bin/taskbar.elf", 0, 0, ELF);

    while(1)
    {
        renderer_clear(0xFF000000);
        update();
        render();
        swap_buffers();
    }

    return 0;
}

static bool mouse_state[3];

void init(void* framebuffer, size_t width, size_t height, size_t pitch)
{
    renderer_init(framebuffer, width, height, pitch);
    
    windows_init();
    memset(mouse_state, 0, sizeof(mouse_state));

    tga_load(&cursor_tga, "a:/assets/cursor.tga");
}

void update()
{
    msg_t msg;
    uint64_t sender = msg_fetch(&msg);
    if(sender != 0)
        msg_process(sender, &msg);

    window_t* active = window_get_focused();
    
    if(is_key_pressed(VK_MOUSE_LEFT) != mouse_state[0])
    {
        mouse_state[0] = !mouse_state[0];
        if(mouse_state[0])
            event_send(active, EVENT_MOUSE_LEFT_DOWN);
        else
            event_send(active, EVENT_MOUSE_LEFT_UP);
    }
    if(is_key_pressed(VK_MOUSE_RIGHT) != mouse_state[1])
    {
        mouse_state[1] = !mouse_state[1];
        if(mouse_state[1])
            event_send(active, EVENT_MOUSE_RIGHT_DOWN);
        else
            event_send(active, EVENT_MOUSE_RIGHT_UP);
    }
}

void render_buffer(window_t* window)
{
    size_t width = window->width;
    if(width + window->x > display_width())
        width = display_width();
    
    size_t height = window->height;
    if(height + window->y > display_height())
        height = display_height();
    
    size_t w = window->width;
    uint32_t* buffer = (uint32_t*)window->framebuffer;
    for(size_t y = 0; y < height; y++)
    {
        size_t y_off = window->y + y;
        for(size_t x = 0; x < width; x++)
            renderer_put_pixel(window->x + x, y_off, buffer[y * w + x]);
    }
        
}

void render()
{
    window_t* windows = get_windows();
    for(size_t i = 0; i < MAX_WINDOW; i++)
    {
        window_t* window = &windows[i];
        if(window->id != -1 && window->framebuffer != NULL)
            render_buffer(window);
    }

    uint64_t mx, my;
    get_mouse(&mx, &my);
    draw_image(&cursor_tga, mx, my);
}