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

int main(int argc, char* argv[])
{
    renderer_init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    init();

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

void init()
{
    windows_init();
    memset(mouse_state, 0, sizeof(mouse_state));
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
}

void render()
{
    window_t* windows = get_windows();
    for(size_t i = 0; i < MAX_WINDOW; i++)
    {
        window_t* window = &windows[i];
        if(window->id != -1 && window->framebuffer != NULL)
        {
            for(size_t x = 0; x < window->width; x++)
                for(size_t y = 0; y < window->height; y++)
                    renderer_put_pixel(window->x + x, window->y + y, ((uint32_t*)window->framebuffer)[x + y * window->width]);
        }
    }
}