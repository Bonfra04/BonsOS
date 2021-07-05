#pragma once

#include <stdint.h>
#include <stddef.h>

void renderer_init(void* framebuffer, size_t width, size_t height, uint64_t pitch);
void renderer_clear(uint32_t color);
void renderer_put_pixel(uint64_t x, uint64_t y, uint32_t color);