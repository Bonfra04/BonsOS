#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct screenpos
{
    uint64_t x;
    uint64_t y;
} tty_pos_t;

/**
 * @brief initializes the tty
 */
void tty_init();

/**
 * @brief returns the current foreground color
 * @return current foreground color RGB
 */
uint32_t tty_get_textcolor_fg();

/**
 * @brief returns the current background color
 * @return current background color RGB
 */
uint32_t tty_get_textcolor_bg();

/**
 * @brief sets the printing colors
 * @param[in] fg foreground color
 * @param[in] bg background color
 */
void tty_set_textcolor(uint32_t fg, uint32_t bg);

/**
 * @brief sets the foreground color
 * @param[in] fg foreground color
 */
void tty_set_textcolor_fg(uint32_t fg);

/**
 * @brief sets the background color
 * @param[in] bg background color
 */
void tty_set_textcolor_bg(uint32_t bg);

/**
 * @brief returns the position of the cursor
 * @return position of the cursor
 */
tty_pos_t tty_getpos();

/**
 * @brief sets the position of the cursor
 * @param[in] tty_pos x,y struct position
 */
void tty_setpos(tty_pos_t tty_pos);

/**
 * @brief clears the screen
 */
void tty_clear();

/**
 * @brief prints a string to the screen
 * @param[in] str string to print
 */
void tty_print(const char* str);

/**
 * @brief reads uncooked characters from the keyboard
 * @return uncooked character
 */
uint8_t tty_read_raw();

/**
 * @brief reads an entire line from the screen and returns the first size characters
 * @param[in] buf buffer to store the line
 * @param[in] size maximum number of characters to read
 */
size_t tty_read(char* buf, size_t size);

