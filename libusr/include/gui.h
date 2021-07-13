#pragma once

#include <stddef.h>
#include <stdint.h>

#define WND_FLAGS_DEFAULT   0b00000000
#define WND_NO_DECORATIONS  0b00000001

typedef struct window
{
    uint64_t id;
    void* framebuffer;
} window_t;

size_t display_width();
size_t display_height();

void window_move(window_t window, size_t x, size_t y);
void* window_resize(window_t window, size_t width, size_t height);
void window_set_title(window_t window, const char* title);
window_t create_window(const char* title, size_t width, size_t height, size_t x, size_t y, uint8_t flags);