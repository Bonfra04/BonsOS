#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_WINDOW 32

typedef struct window
{
    uint64_t id;
    size_t x, y;
    size_t width, height;
    uint8_t flags;
    void* framebuffer;
    char title[32];
} window_t;

void windows_init();

uint64_t window_create(uint64_t flags);
void window_move(uint64_t window_id, size_t x, size_t y);
void* window_resize(uint64_t window_id, size_t width, size_t height);
void window_set_title(uint64_t window_id, const char* title);

window_t* get_windows();
