#include <io/keyboard.h>

const kb_layout_t kb_layout_en_us = {
    [0x00 ... UINT16_MAX - 1] = { .keycode = UINT16_MAX, .state = false },

    [0x29] = { .keycode = KEY_BACKTICK,      .state = true },
    [0x02] = { .keycode = KEY_1,             .state = true },
    [0x03] = { .keycode = KEY_2,             .state = true },
    [0x04] = { .keycode = KEY_3,             .state = true },
    [0x05] = { .keycode = KEY_4,             .state = true },
    [0x06] = { .keycode = KEY_5,             .state = true },
    [0x07] = { .keycode = KEY_6,             .state = true },
    [0x08] = { .keycode = KEY_7,             .state = true },
    [0x09] = { .keycode = KEY_8,             .state = true },
    [0x0A] = { .keycode = KEY_9,             .state = true },
    [0x0B] = { .keycode = KEY_0,             .state = true },
    [0x0C] = { .keycode = KEY_MINUS,         .state = true },
    [0x0D] = { .keycode = KEY_EQUALS,        .state = true },
    [0x0E] = { .keycode = KEY_BACKSPACE,     .state = true },
    [0x0F] = { .keycode = KEY_TAB,           .state = true },
    [0x10] = { .keycode = KEY_Q,             .state = true },
    [0x11] = { .keycode = KEY_W,             .state = true },
    [0x12] = { .keycode = KEY_E,             .state = true },
    [0x13] = { .keycode = KEY_R,             .state = true },
    [0x14] = { .keycode = KEY_T,             .state = true },
    [0x15] = { .keycode = KEY_Y,             .state = true },
    [0x16] = { .keycode = KEY_U,             .state = true },
    [0x17] = { .keycode = KEY_I,             .state = true },
    [0x18] = { .keycode = KEY_O,             .state = true },
    [0x19] = { .keycode = KEY_P,             .state = true },
    [0x1A] = { .keycode = KEY_LEFT_BRACKET,  .state = true },
    [0x1B] = { .keycode = KEY_RIGHT_BRACKET, .state = true },
    [0x1C] = { .keycode = KEY_ENTER,         .state = true },
    [0x3A] = { .keycode = KEY_CAPS_LOCK,     .state = true },
    [0x1E] = { .keycode = KEY_A,             .state = true },
    [0x1F] = { .keycode = KEY_S,             .state = true },
    [0x20] = { .keycode = KEY_D,             .state = true },
    [0x21] = { .keycode = KEY_F,             .state = true },
    [0x22] = { .keycode = KEY_G,             .state = true },
    [0x23] = { .keycode = KEY_H,             .state = true },
    [0x24] = { .keycode = KEY_J,             .state = true },
    [0x25] = { .keycode = KEY_K,             .state = true },
    [0x26] = { .keycode = KEY_L,             .state = true },
    [0x27] = { .keycode = KEY_SEMICOLON,     .state = true },
    [0x28] = { .keycode = KEY_APOSTROPHE,    .state = true },
    [0x2B] = { .keycode = KEY_BACKSLASH,     .state = true },
    [0x2A] = { .keycode = KEY_LEFT_SHIFT,    .state = true },
    [0x2C] = { .keycode = KEY_Z,             .state = true },
    [0x2D] = { .keycode = KEY_X,             .state = true },
    [0x2E] = { .keycode = KEY_C,             .state = true },
    [0x2F] = { .keycode = KEY_V,             .state = true },
    [0x30] = { .keycode = KEY_B,             .state = true },
    [0x31] = { .keycode = KEY_N,             .state = true },
    [0x32] = { .keycode = KEY_M,             .state = true },
    [0x33] = { .keycode = KEY_COMMA,         .state = true },
    [0x34] = { .keycode = KEY_PERIOD,        .state = true },
    [0x35] = { .keycode = KEY_FORWARD_SLASH, .state = true },
    [0x36] = { .keycode = KEY_RIGHT_SHIFT,   .state = true },
    [0x1D] = { .keycode = KEY_LEFT_CONTROL,  .state = true },
    [0x38] = { .keycode = KEY_LEFT_ALT,      .state = true },
    [0x39] = { .keycode = KEY_SPACE,         .state = true },
    [0x3B] = { .keycode = KEY_RIGHT_ALT,     .state = true },
    [0x3C] = { .keycode = KEY_RIGHT_CONTROL, .state = true }, // TODO: check with us keyboard

    [0xA9] = { .keycode = KEY_BACKTICK,      .state = false },
    [0x82] = { .keycode = KEY_1,             .state = false },
    [0x83] = { .keycode = KEY_2,             .state = false },
    [0x84] = { .keycode = KEY_3,             .state = false },
    [0x85] = { .keycode = KEY_4,             .state = false },
    [0x86] = { .keycode = KEY_5,             .state = false },
    [0x87] = { .keycode = KEY_6,             .state = false },
    [0x88] = { .keycode = KEY_7,             .state = false },
    [0x89] = { .keycode = KEY_8,             .state = false },
    [0x8A] = { .keycode = KEY_9,             .state = false },
    [0x8B] = { .keycode = KEY_0,             .state = false },
    [0x8C] = { .keycode = KEY_MINUS,         .state = false },
    [0x8D] = { .keycode = KEY_EQUALS,        .state = false },
    [0x8E] = { .keycode = KEY_BACKSPACE,     .state = false },
    [0x8F] = { .keycode = KEY_TAB,           .state = false },
    [0x90] = { .keycode = KEY_Q,             .state = false },
    [0x91] = { .keycode = KEY_W,             .state = false },
    [0x92] = { .keycode = KEY_E,             .state = false },
    [0x93] = { .keycode = KEY_R,             .state = false },
    [0x94] = { .keycode = KEY_T,             .state = false },
    [0x95] = { .keycode = KEY_Y,             .state = false },
    [0x96] = { .keycode = KEY_U,             .state = false },
    [0x97] = { .keycode = KEY_I,             .state = false },
    [0x98] = { .keycode = KEY_O,             .state = false },
    [0x99] = { .keycode = KEY_P,             .state = false },
    [0x9A] = { .keycode = KEY_LEFT_BRACKET,  .state = false },
    [0x9B] = { .keycode = KEY_RIGHT_BRACKET, .state = false },
    [0xBA] = { .keycode = KEY_CAPS_LOCK,     .state = false },
    [0x9E] = { .keycode = KEY_A,             .state = false },
    [0x9F] = { .keycode = KEY_S,             .state = false },
    [0xA0] = { .keycode = KEY_D,             .state = false },
    [0xA1] = { .keycode = KEY_F,             .state = false },
    [0xA2] = { .keycode = KEY_G,             .state = false },
    [0xA3] = { .keycode = KEY_H,             .state = false },
    [0xA4] = { .keycode = KEY_J,             .state = false },
    [0xA5] = { .keycode = KEY_K,             .state = false },
    [0xA6] = { .keycode = KEY_L,             .state = false },
    [0xA7] = { .keycode = KEY_SEMICOLON,     .state = false },
    [0xA8] = { .keycode = KEY_APOSTROPHE,    .state = false },
    [0xAB] = { .keycode = KEY_BACKSLASH,     .state = false },
    [0xAA] = { .keycode = KEY_LEFT_SHIFT,    .state = false },
    [0xAC] = { .keycode = KEY_Z,             .state = false },
    [0xAD] = { .keycode = KEY_X,             .state = false },
    [0xAE] = { .keycode = KEY_C,             .state = false },
    [0xAF] = { .keycode = KEY_V,             .state = false },
    [0xB0] = { .keycode = KEY_B,             .state = false },
    [0xB1] = { .keycode = KEY_N,             .state = false },
    [0xB2] = { .keycode = KEY_M,             .state = false },
    [0xB3] = { .keycode = KEY_COMMA,         .state = false },
    [0xB4] = { .keycode = KEY_PERIOD,        .state = false },
    [0xB5] = { .keycode = KEY_FORWARD_SLASH, .state = false },
    [0xB6] = { .keycode = KEY_RIGHT_SHIFT,   .state = false },
    [0x9D] = { .keycode = KEY_LEFT_CONTROL,  .state = false }, // TODO: check with us keyboard
};
