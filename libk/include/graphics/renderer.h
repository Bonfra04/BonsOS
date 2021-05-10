#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool renderer_load_font(const char* filepath);
void renderer_putchar(size_t x, size_t y, char c, uint32_t fg, uint32_t bg);
size_t renderer_charwidth();
size_t renderer_charheight();