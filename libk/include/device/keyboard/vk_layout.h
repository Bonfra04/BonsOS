#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct key_state
{
    uint8_t keycode;
    bool isDown;
} key_state_t;

typedef struct keyboard_state
{
    bool lShiftDown;
    bool rShiftDown;
    bool lCtrlDown;
    bool rCtrlDown;
    bool lAltDown;
    bool rAltDown;
    bool isCapsLock;
} keyboard_state_t;

// ################
// ## First half ##
// ################

// First Line
#define VK_ESCAPE               0x00
#define VK_F1                   0x01
#define VK_F2                   0x02
#define VK_F3                   0x03
#define VK_F4                   0x04
#define VK_F5                   0x05
#define VK_F6                   0x06
#define VK_F7                   0x07
#define VK_F8                   0x08
#define VK_F9                   0x09
#define VK_F10                  0x0A
#define VK_F11                  0x0B
#define VK_F12                  0x0C


// Second line
#define VK_BACK_TICK            0x0D
#define VK_1                    0x0E
#define VK_2                    0x0F
#define VK_3                    0x10
#define VK_4                    0x11
#define VK_5                    0x12
#define VK_6                    0x13
#define VK_7                    0x14
#define VK_8                    0x15
#define VK_9                    0x16
#define VK_0                    0x17
#define VK_MINUS                0x18
#define VK_EQUALS               0x19
#define VK_BACKSPACE            0x1A

// Third line
#define VK_TAB                  0x1B
#define VK_Q                    0x1C
#define VK_W                    0x1D
#define VK_E                    0x1E
#define VK_R                    0x1F
//#define VK_E                    0x20
#define VK_T                    0x21
#define VK_Y                    0x22
#define VK_U                    0x23
#define VK_I                    0x24
#define VK_O                    0x25
#define VK_P                    0x26
#define VK_OPEN_SQUARE_BRACKET  0x27
#define VK_CLOSE_SQUARE_BRACKET 0x28
#define VK_BACK_SLASH           0x29

// Fourth line
#define VK_CAPS_LOCK            0x2A
#define VK_A                    0x2B
#define VK_S                    0x2C
#define VK_D                    0x2D
#define VK_F                    0x2E
#define VK_G                    0x2F
#define VK_H                    0x30
#define VK_J                    0x31
#define VK_K                    0x32
#define VK_L                    0x33
#define VK_SEMICOLON            0x34
#define VK_SINGLE_QUOTE         0x35
#define VK_ENTER                0x36

// Fifth line
#define VK_RIGHT_SHIFT          0x37
#define VK_Z                    0x38
#define VK_X                    0x39
#define VK_C                    0x3A
#define VK_V                    0x3B
#define VK_B                    0x3C
#define VK_N                    0x3D
#define VK_M                    0x3E
#define VK_COMMA                0x3F
#define VK_PERIOD               0x40
#define VK_FORWARD_SLASH        0x41
#define VK_LEFT_SHIFT           0x42

// Sixth line
#define VK_LEFT_CTRL            0x43
#define VK_LEFT_GUI             0x44
#define VK_LEFT_ALT             0x45
#define VK_SPACE                0x46
#define VK_RIGHT_ALT            0x47
#define VK_RIGHT_GUI            0x48
#define VK_MENU                 0x49
#define VK_RIGHT_CTRL           0x4A

// ############
// ## Arrows ##
// ############
#define VK_ARROW_UP             0x60
#define VK_ARROW_LEFT           0x61
#define VK_ARROW_DOWN           0x62
#define VK_ARROW_RIGHT          0x63

// ###########
// ## Utils ##
// ###########
#define VK_INSERT                 0x64
#define VK_DELETE                 0x65
#define VK_HOME                   0x66
#define VK_END                    0x67
#define VK_PAGE_UP                0x68
#define VK_PAGE_DOWN              0x69
#define VK_SCROLL_LOCK            0x6A
#define VK_PAUSE                  0x7C
#define VK_PRINT_SCREEN           0x7D

// ############
// ## Keypad ##
// ############
#define VK_NUMBER_LOCK            0x6B
#define VK_KEYPAD_FORWARD_SLASH   0x6C
#define VK_KEYPAD_ASTERISK        0x6D
#define VK_KEYPAD_MINUS           0x6E
#define VK_KEYPAD_PLUS            0x6F
#define VK_KEYPAD_PERIOD          0x70
#define VK_KEYPAD_7               0x71
#define VK_KEYPAD_8               0x72
#define VK_KEYPAD_9               0x73
#define VK_KEYPAD_4               0x74
#define VK_KEYPAD_5               0x75
#define VK_KEYPAD_6               0x76
#define VK_KEYPAD_1               0x77
#define VK_KEYPAD_2               0x78
#define VK_KEYPAD_3               0x79
#define VK_KEYPAD_0               0x7A
#define VK_KEYPAD_ENTER           0x7B

// Shifted
#define VK_TILDE                  0x7E
#define VK_EXCLAMATION_MARK       0x7f
#define VK_AT                     0x80
#define VK_NUMBER_SIGN            0x81
#define VK_DOLLAR                 0x82
#define VK_PERCENT                0x83
#define VK_CIRCUMFLEX             0x84
#define VK_AND                    0x85
#define VK_ASTERISK               0x86
#define VK_OPEN_PARENTHESIS       0x87
#define VK_CLOSE_PARENTHESIS      0x88
#define VK_UNDER_SCORE            0x89
#define VK_PLUS                   0x8A

#define VK_OPEN_CURLY_BRACKET     0x8B
#define VK_CLOSE_CURLY_BRACKET    0x8C
#define VK_PIPE                   0x8D
#define VK_COLON                  0x8E
#define VK_DOUBLE_QUOTE           0x8F
#define VK_LESS                   0x90
#define VK_GREATER                0x91
#define VK_QUESTION_MARK          0x92