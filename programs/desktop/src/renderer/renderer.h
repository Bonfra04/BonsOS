#pragma once

#include <stdint.h>
#include <stddef.h>
#include "tga.h"

void renderer_init(void* framebuffer, size_t width, size_t height, uint64_t pitch);
void renderer_clear(uint32_t color);
void renderer_put_pixel(uint64_t x, uint64_t y, uint32_t color);
void fill_rect(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t color);
void draw_image(tga_t* image, uint64_t x, uint64_t y);

uint64_t display_width();
uint64_t display_height();

void swap_buffers();
