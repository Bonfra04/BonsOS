#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <io/keycodes.h>

typedef uint16_t kb_layout_t[UINT16_MAX];
extern const kb_layout_t kb_layout_en_us;

/**
 * @brief initializes the keyboard
 */
void keyboard_init();

/**
 * @brief returns the state of a physical key
 * @param key the physical key to check
 * @return true if the key is pressed, false otherwise
 */
bool keyboard_get_key(uint16_t key);

/**
 * @brief sets the layout of the keyboard
 * @param layout the layout to set
 */
void keyboard_layout_set(const kb_layout_t layout);
