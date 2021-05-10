#pragma once

#include <stddef.h>

void screen_init(size_t width, size_t height, size_t pitch, void* framebuffer);

size_t screen_get_width();
size_t screen_get_height();
size_t screen_get_pitch();

/**
 * @brief clears the scren with a specific color
 * @param[in] color ARGB color to clear with
 */ 
void screen_clear(int color);

/**
 * @brief plots a colored pixel to the screen
 * @param[in] x x coordinate
 * @param[in] y y coordinate
 * @param[in] color ARGB color of the pixel 
 */ 
void screen_plot_pixel(size_t x, size_t y, int color);