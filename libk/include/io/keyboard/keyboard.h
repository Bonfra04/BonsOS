#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <io/keyboard/keycodes.h>

typedef struct kb_layout
{
    uint16_t physical[UINT16_MAX];
    struct
    {
        uint16_t vk_normal;
        uint16_t vk_shift;
        uint16_t vk_altgr;
        uint16_t vk_shift_altgr;
    } virtual[UINT16_MAX];
} kb_layout_t;
extern const kb_layout_t kb_layout_en_us;

typedef union keyevent
{
    struct
    {
        uint16_t keycode;
        bool is_pressed;
    };
    uint32_t value;
} keyevent_t;

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
void keyboard_layout_set(const kb_layout_t* layout);

/**
 * @brief pulls a key event from a queue
 * @return the keycode of the key event, waits until a key event is available
 */
keyevent_t keyboard_pull();

/**
 * @brief checks weather a key event is available
 * @return true if a key event is available, false otherwise
 */
bool keyboard_has_event();

/**
 * @brief flushes the key event queue, discarding all keypresses
 */
void keyboard_flush();

/**
 * @brief returns a virtual key combining the provided physical key with the current modifier keys
 * @param physical_key the physical key to combine
 */
uint16_t keyboard_translate_vk(uint16_t physical_key);
