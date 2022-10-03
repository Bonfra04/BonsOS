#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum escape_type
{
    ESC_SS2 = 'N', ESC_SS3 = 'O', ESC_DCS = 'P', ESC_CSI = '[', ESC_ST = '\\',
    ESC_OSC = ']', ESC_SOS = 'X', ESC_PM = '^', ESC_APC = '_',
    ESC_NONE = 0
} escape_type_t;

typedef struct escape
{
    escape_type_t type;
    union
    {
        uint8_t data[17];
        struct
        {
            uint8_t type;
            uint64_t param0;
            uint64_t param1;
        } csi;
    };
} escape_t;

typedef enum escpale_keyboard
{
    ESCKB_ARROW_LEFT, ESCKB_ARROW_RIGHT, ESCKB_ARROW_UP, ESCKB_ARROW_DOWN,
    ESCKB_NONE,
} escape_keyboard_t;

escape_t readescape();
void printescape(const escape_t* escape);

escape_keyboard_t escape_simpify(const escape_t* escape);
