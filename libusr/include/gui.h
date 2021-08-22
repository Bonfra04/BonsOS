#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define WM_PID ((1ull << 56ull) | 2ull)

#define WND_FLAGS_DEFAULT   0b00000000
#define WND_NO_DECORATIONS  0b00000001

#define EVENT_MOUSE_LEFT_UP     0x00
#define EVENT_MOUSE_LEFT_DOWN   0x01
#define EVENT_MOUSE_RIGHT_UP    0x02
#define EVENT_MOUSE_RIGHT_DOWN  0x03
#define EVENT_MOUSE_MIDDLE_UP   0x04
#define EVENT_MOUSE_MIDDLE_DOWN 0x05

typedef struct window
{
    uint64_t id;
    void* framebuffer;
    uint64_t width;
    uint64_t height;
} window_t;

size_t display_width();
size_t display_height();

void window_move(window_t window, size_t x, size_t y);
void* window_resize(window_t window, size_t width, size_t height);
void window_set_title(window_t window, const char* title);
window_t window_create(const char* title, size_t width, size_t height, size_t x, size_t y, uint8_t flags);

typedef void(*event_handler_t)(uint64_t wnd_id, uint8_t event_id);

bool cycle_events(event_handler_t handler);
void default_event_handler(uint64_t wnd_id, uint8_t event_id);