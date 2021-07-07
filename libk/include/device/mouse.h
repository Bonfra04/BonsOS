#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BTN_LEFT 0x00
#define BTN_RIGHT 0x01
#define BTN_MIDDLE 0x02

void mouse_init(uint64_t boundX, uint64_t boundY);

bool mouse_is_down(uint8_t btn);

uint64_t mouse_get_x();
uint64_t mouse_get_y();