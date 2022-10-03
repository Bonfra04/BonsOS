#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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
 * @brief prints a stream of bytes to the tty
 * @param[in] buff stream of bytes to print
 * @param[in] raw flag signaling if the bytes should be cooked
 */
void tty_print(const char* buff, bool raw);

/**
 * @brief reads a stream of bytes from the tty
 * @param[out] buff buffer to read into
 * @param[in] count number of bytes to read 
 * @param[in] raw flag signaling if the bytes should be cooked
 * @return number of bytes read
 */
size_t tty_read(char* buff, size_t count, bool raw);

