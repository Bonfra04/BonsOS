#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief initializes the screen
 * @param[in] width the width of the screen
 * @param[in] height the height of the screen
 * @param[in] pitch the pitch of the screen
 * @param[in] framebuffer framebuffer address
 */
void screen_init(size_t width, size_t height, size_t pitch, void* framebuffer);

/**
 * @brief returns the width of the screen
 * @return the width of the screen
 */
size_t screen_get_width();

/**
 * @brief returns the height of the screen
 * @return the height of the screen
 */
size_t screen_get_height();

/**
 * @brief returns the pitch of the screen
 * @return the pitch of the screen
 */
size_t screen_get_pitch();

/**
 * @brief clears the screen with a specific color
 * @param[in] color RGB color to clear with
 */
void screen_clear(uint32_t color);

/**
 * @brief plots a colored pixel to the screen
 * @param[in] x x coordinate
 * @param[in] y y coordinate
 * @param[in] color RGB color of the pixel
 */
void screen_plot_pixel(uint64_t x, uint64_t y, uint32_t color);

/**
 * @brief reads a pixel value from the screen
 * @param[in] x x coordinate
 * @param[in] y y coordinate
 * @return RGB color of the pixel
 */
uint32_t screen_get_pixel(uint64_t x, uint64_t y);

/**
 * @brief sets a screen area to the colors specified in the given array
 * @param[in] x x coordinate of the start of the area
 * @param[in] y y coordinate of the start of the area
 * @param[in] size number of contiguous pixels from the coordinates on
 * @param[in] pixels array of pixels (RGB format) to copy to the screen
 * @return number of pixels correctly written
 */
size_t screen_set_area(uint64_t x, uint64_t y, uint64_t size, const uint32_t* pixels);

/**
 * @brief reads a screen area to the specified pixels array
 * @param[in] x x coordinate of the start of the area
 * @param[in] y y coordinate of the start of the area
 * @param[in] size number of contiguous pixels from the coordinates on
 * @param[in] pixels array of pixels (RGB format) to populate
 * @return number of pixels correctly read
 */
size_t screen_get_area(uint64_t x, uint64_t y, uint64_t size, uint32_t* pixels);
