#include "window.h"
#include <string.h>
#include <stdlib.h>
#include <syscalls.h>

static window_t windows[MAX_WINDOW];

void windows_init()
{
    memset(windows, -1, sizeof(window_t) * MAX_WINDOW);
}

uint64_t window_create(uint64_t flags)
{
    window_t* window = NULL;
    for(size_t i = 0; i < MAX_WINDOW; i++)
        if(windows[i].id == -1)
        {
            window = &windows[i];
            window->id = i;
            break;
        }
    if(window == NULL)
        return -1;

    window->flags = flags;
    window->x = 0;
    window->y = 0;
    window->width = 0;
    window->height = 0;
    window->framebuffer = 0;
    memset(window->title, 0, sizeof(window->title));

    return window->id;
}

void window_move(uint64_t window_id, size_t x, size_t y)
{
    if(window_id >= MAX_WINDOW)
        return;
    window_t* window = &windows[window_id];
    window->x = x;
    window->y = y;
}

void* window_resize(uint64_t window_id, size_t width, size_t height)
{
    if(window_id >= MAX_WINDOW)
        return;
    window_t* window = &windows[window_id];
    if(window->framebuffer != NULL)
        free(window->framebuffer);
    window->framebuffer = map_mem(0, width * height * sizeof(uint32_t));
    memset(window->framebuffer, 0, width * height * sizeof(uint32_t));
    window->width = width;
    window->height = height;

    return window->framebuffer;
}

void window_set_title(uint64_t window_id, const char* title)
{
    if(window_id >= MAX_WINDOW)
        return;
    window_t* window = &windows[window_id];
    strcpy(window->title, title);
}

window_t* get_windows()
{
    return windows;
}
