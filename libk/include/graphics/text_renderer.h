#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief initializes the text renderer with the default font
 */
void text_renderer_init();

/**
 * @brief loads a font from a file
 * @param[in] filepath the path of the font file
 * @return true if the font was loaded successfully, false otherwise
 */
bool text_renderer_load_font(const char* filepath);

/**
 * @brief renders a character on the screen
 * @param[in] x x coordinate
 * @param[in] y y coordinate
 * @param[in] scale the scale of the character
 * @param[in] c the character to render
 * @param[in] fg RGB color of the foreground
 * @param[in] bg RGB color of the background
 */
void text_renderer_putchar(size_t x, size_t y, size_t scale, char c, uint32_t fg, uint32_t bg);

/**
 * @brief returns the with of a character at a specific scale
 * @param[in] scale the scale of the character
 * @return the width of the character
 */
size_t text_renderer_charwidth(size_t scale);

/**
 * @brief returns the height of a character at a specific scale
 * @param[in] scale the scale of the character
 * @return the height of the character
 */
size_t text_renderer_charheight(size_t scale);
