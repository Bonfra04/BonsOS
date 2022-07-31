#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <io/keycodes.h>

typedef struct vk_entry
{
    uint16_t vk;
    uint8_t mods;
} vk_entry_t;

#define VK_MOD_SHIFT   0x01
#define VK_MOD_CTRL    0x02
#define VK_MOD_ALT     0x04
#define VK_MOD_MAIUSC  0x08

typedef struct kb_layout_entry
{
    uint16_t physical;
    vk_entry_t altered[3];
} kb_layout_entry_t;

typedef kb_layout_entry_t kb_layout_t[UINT16_MAX];

extern const kb_layout_t kb_layout_en_us;

typedef union keyevent
{
    struct
    {
        uint16_t ph_keycode;
        uint16_t vt_keycode;
        bool is_pressed;
    };
    uint64_t value;
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
 * @brief returns the state of a virtual key
 * @param key the virtual key to check
 * @return true if the key is pressed, false otherwise
 */
bool keyboard_get_vkey(uint16_t key);

/**
 * @brief sets the layout of the keyboard
 * @param layout the layout to set
 */
void keyboard_layout_set(const kb_layout_t* layout);

/**
 * @brief pulls a keyevent from a queue
 * @return the keycode of the keyevent, waits until a keyevent is available
 */
keyevent_t keyboard_pull();

/**
 * @brief flushes the keyevent queue, discarding all keypresses
 */
void keyboard_flush();
