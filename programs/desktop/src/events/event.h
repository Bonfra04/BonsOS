#pragma once

#include "../window/window.h"

#define EVENT_MOUSE_LEFT_UP     0x00
#define EVENT_MOUSE_LEFT_DOWN   0x01
#define EVENT_MOUSE_RIGHT_UP    0x02
#define EVENT_MOUSE_RIGHT_DOWN  0x03
#define EVENT_MOUSE_MIDDLE_UP   0x04
#define EVENT_MOUSE_MIDDLE_DOWN 0x05

void event_send(const window_t* window, uint8_t event_id);