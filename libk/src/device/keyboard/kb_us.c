#include <device/keyboard/kb_us.h>

key_state_t physical_to_virtual_key(uint16_t physical)
{
    switch (physical)
    {
    case KEY_PRESSED_ESCAPE:                 return (key_state_t){ VK_ESCAPE, true };
    case KEY_PRESSED_1:                      return (key_state_t){ VK_1, true };
    case KEY_PRESSED_2:                      return (key_state_t){ VK_2, true };
    case KEY_PRESSED_3:                      return (key_state_t){ VK_3, true };
    case KEY_PRESSED_4:                      return (key_state_t){ VK_4, true };
    case KEY_PRESSED_5:                      return (key_state_t){ VK_5, true };
    case KEY_PRESSED_6:                      return (key_state_t){ VK_6, true };
    case KEY_PRESSED_7:                      return (key_state_t){ VK_7, true };
    case KEY_PRESSED_8:                      return (key_state_t){ VK_8, true };
    case KEY_PRESSED_9:                      return (key_state_t){ VK_9, true };
    case KEY_PRESSED_0:                      return (key_state_t){ VK_0, true };
    case KEY_PRESSED_MINUS:                  return (key_state_t){ VK_MINUS, true };
    case KEY_PRESSED_EQUALS:                 return (key_state_t){ VK_EQUALS, true };
    case KEY_PRESSED_BACKSPACE:              return (key_state_t){ VK_BACKSPACE, true };
    case KEY_PRESSED_TAB:                    return (key_state_t){ VK_TAB, true };
    case KEY_PRESSED_Q:                      return (key_state_t){ VK_Q, true };
    case KEY_PRESSED_W:                      return (key_state_t){ VK_W, true };
    case KEY_PRESSED_E:                      return (key_state_t){ VK_E, true };
    case KEY_PRESSED_R:                      return (key_state_t){ VK_R, true };
    case KEY_PRESSED_T:                      return (key_state_t){ VK_T, true };
    case KEY_PRESSED_Y:                      return (key_state_t){ VK_Y, true };
    case KEY_PRESSED_U:                      return (key_state_t){ VK_U, true };
    case KEY_PRESSED_I:                      return (key_state_t){ VK_I, true };
    case KEY_PRESSED_O:                      return (key_state_t){ VK_O, true };
    case KEY_PRESSED_P:                      return (key_state_t){ VK_P, true };
    case KEY_PRESSED_OPEN_SQUARE_BRACKET:    return (key_state_t){ VK_OPEN_SQUARE_BRACKET, true };
    case KEY_PRESSED_CLOSE_SQUARE_BRACKET:   return (key_state_t){ VK_CLOSE_SQUARE_BRACKET, true };
    case KEY_PRESSED_ENTER:                  return (key_state_t){ VK_ENTER, true };
    case KEY_PRESSED_LEFT_CTRL:              return (key_state_t){ VK_LEFT_CTRL, true };
    case KEY_PRESSED_A:                      return (key_state_t){ VK_A, true };
    case KEY_PRESSED_S:                      return (key_state_t){ VK_S, true };
    case KEY_PRESSED_D:                      return (key_state_t){ VK_D, true };
    case KEY_PRESSED_F:                      return (key_state_t){ VK_F, true };
    case KEY_PRESSED_G:                      return (key_state_t){ VK_G, true };
    case KEY_PRESSED_H:                      return (key_state_t){ VK_H, true };
    case KEY_PRESSED_J:                      return (key_state_t){ VK_J, true };
    case KEY_PRESSED_K:                      return (key_state_t){ VK_K, true };
    case KEY_PRESSED_L:                      return (key_state_t){ VK_L, true };
    case KEY_PRESSED_SEMICOLON:              return (key_state_t){ VK_SEMICOLON, true };
    case KEY_PRESSED_SINGLE_QUOTE:           return (key_state_t){ VK_SINGLE_QUOTE, true };
    case KEY_PRESSED_BACK_TICK:              return (key_state_t){ VK_BACK_TICK, true };
    case KEY_PRESSED_LEFT_SHIFT:             return (key_state_t){ VK_LEFT_SHIFT, true };
    case KEY_PRESSED_BACK_SLASH:             return (key_state_t){ VK_BACK_SLASH, true };
    case KEY_PRESSED_Z:                      return (key_state_t){ VK_Z, true };
    case KEY_PRESSED_X:                      return (key_state_t){ VK_X, true };
    case KEY_PRESSED_C:                      return (key_state_t){ VK_C, true };
    case KEY_PRESSED_V:                      return (key_state_t){ VK_V, true };
    case KEY_PRESSED_B:                      return (key_state_t){ VK_B, true };
    case KEY_PRESSED_N:                      return (key_state_t){ VK_N, true };
    case KEY_PRESSED_M:                      return (key_state_t){ VK_M, true };
    case KEY_PRESSED_COMMA:                  return (key_state_t){ VK_COMMA, true };
    case KEY_PRESSED_PERIOD:                 return (key_state_t){ VK_PERIOD, true };
    case KEY_PRESSED_FORWARD_SLASH:          return (key_state_t){ VK_FORWARD_SLASH, true };
    case KEY_PRESSED_RIGHT_SHIFT:            return (key_state_t){ VK_RIGHT_SHIFT, true };
    case KEY_PRESSED_KEYPAD_ASTERISK:        return (key_state_t){ VK_KEYPAD_ASTERISK, true };
    case KEY_PRESSED_LEFT_ALT:               return (key_state_t){ VK_LEFT_ALT, true };
    case KEY_PRESSED_SPACE:                  return (key_state_t){ VK_SPACE, true };
    case KEY_PRESSED_CAPS_LOCK:              return (key_state_t){ VK_CAPS_LOCK, true };
    case KEY_PRESSED_F1:                     return (key_state_t){ VK_F1, true };
    case KEY_PRESSED_F2:                     return (key_state_t){ VK_F2, true };
    case KEY_PRESSED_F3:                     return (key_state_t){ VK_F3, true };
    case KEY_PRESSED_F4:                     return (key_state_t){ VK_F4, true };
    case KEY_PRESSED_F5:                     return (key_state_t){ VK_F5, true };
    case KEY_PRESSED_F6:                     return (key_state_t){ VK_F6, true };
    case KEY_PRESSED_F7:                     return (key_state_t){ VK_F7, true };
    case KEY_PRESSED_F8:                     return (key_state_t){ VK_F8, true };
    case KEY_PRESSED_F9:                     return (key_state_t){ VK_F9, true };
    case KEY_PRESSED_F10:                    return (key_state_t){ VK_F10, true };
    case KEY_PRESSED_NUMBER_LOCK:            return (key_state_t){ VK_NUMBER_LOCK, true };
    case KEY_PRESSED_SCROLL_LOCK:            return (key_state_t){ VK_SCROLL_LOCK, true };
    case KEY_PRESSED_KEYPAD_7:               return (key_state_t){ VK_KEYPAD_7, true };
    case KEY_PRESSED_KEYPAD_8:               return (key_state_t){ VK_KEYPAD_8, true };
    case KEY_PRESSED_KEYPAD_9:               return (key_state_t){ VK_KEYPAD_9, true };
    case KEY_PRESSED_KEYPAD_MINUS:           return (key_state_t){ VK_KEYPAD_MINUS, true };
    case KEY_PRESSED_KEYPAD_4:               return (key_state_t){ VK_KEYPAD_4, true };
    case KEY_PRESSED_KEYPAD_5:               return (key_state_t){ VK_KEYPAD_5, true };
    case KEY_PRESSED_KEYPAD_6:               return (key_state_t){ VK_KEYPAD_6, true };
    case KEY_PRESSED_KEYPAD_PLUS:            return (key_state_t){ VK_KEYPAD_PLUS, true };
    case KEY_PRESSED_KEYPAD_1:               return (key_state_t){ VK_KEYPAD_1, true };
    case KEY_PRESSED_KEYPAD_2:               return (key_state_t){ VK_KEYPAD_2, true };
    case KEY_PRESSED_KEYPAD_3:               return (key_state_t){ VK_KEYPAD_3, true };
    case KEY_PRESSED_KEYPAD_0:               return (key_state_t){ VK_KEYPAD_0, true };
    case KEY_PRESSED_KEYPAD_PERIOD:          return (key_state_t){ VK_KEYPAD_PERIOD, true };
    case KEY_PRESSED_F11:                    return (key_state_t){ VK_F11, true };
    case KEY_PRESSED_F12:                    return (key_state_t){ VK_F12, true };
    
    case KEY_RELEASED_ESCAPE:                return (key_state_t){ VK_ESCAPE, false };
    case KEY_RELEASED_1:                     return (key_state_t){ VK_1, false };
    case KEY_RELEASED_2:                     return (key_state_t){ VK_2, false };
    case KEY_RELEASED_3:                     return (key_state_t){ VK_3, false };
    case KEY_RELEASED_4:                     return (key_state_t){ VK_4, false };
    case KEY_RELEASED_5:                     return (key_state_t){ VK_5, false };
    case KEY_RELEASED_6:                     return (key_state_t){ VK_6, false };
    case KEY_RELEASED_7:                     return (key_state_t){ VK_7, false };
    case KEY_RELEASED_8:                     return (key_state_t){ VK_8, false };
    case KEY_RELEASED_9:                     return (key_state_t){ VK_9, false };
    case KEY_RELEASED_0:                     return (key_state_t){ VK_0, false };
    case KEY_RELEASED_MINUS:                 return (key_state_t){ VK_MINUS, false };
    case KEY_RELEASED_EQUALS:                return (key_state_t){ VK_EQUALS, false };
    case KEY_RELEASED_BACKSPACE:             return (key_state_t){ VK_BACKSPACE, false };
    case KEY_RELEASED_TAB:                   return (key_state_t){ VK_TAB, false };
    case KEY_RELEASED_Q:                     return (key_state_t){ VK_Q, false };
    case KEY_RELEASED_W:                     return (key_state_t){ VK_W, false };
    case KEY_RELEASED_E:                     return (key_state_t){ VK_E, false };
    case KEY_RELEASED_R:                     return (key_state_t){ VK_R, false };
    case KEY_RELEASED_T:                     return (key_state_t){ VK_T, false };
    case KEY_RELEASED_Y:                     return (key_state_t){ VK_Y, false };
    case KEY_RELEASED_U:                     return (key_state_t){ VK_U, false };
    case KEY_RELEASED_I:                     return (key_state_t){ VK_I, false };
    case KEY_RELEASED_O:                     return (key_state_t){ VK_O, false };
    case KEY_RELEASED_P:                     return (key_state_t){ VK_P, false };
    case KEY_RELEASED_OPEN_SQUARE_BRACKET:   return (key_state_t){ VK_OPEN_SQUARE_BRACKET, false };
    case KEY_RELEASED_CLOSE_SQUARE_BRACKET:  return (key_state_t){ VK_CLOSE_SQUARE_BRACKET, false };
    case KEY_RELEASED_ENTER:                 return (key_state_t){ VK_ENTER, false };
    case KEY_RELEASED_LEFT_CTRL:             return (key_state_t){ VK_LEFT_CTRL, false };
    case KEY_RELEASED_A:                     return (key_state_t){ VK_A, false };
    case KEY_RELEASED_S:                     return (key_state_t){ VK_S, false };
    case KEY_RELEASED_D:                     return (key_state_t){ VK_D, false };
    case KEY_RELEASED_F:                     return (key_state_t){ VK_F, false };
    case KEY_RELEASED_G:                     return (key_state_t){ VK_G, false };
    case KEY_RELEASED_H:                     return (key_state_t){ VK_H, false };
    case KEY_RELEASED_J:                     return (key_state_t){ VK_J, false };
    case KEY_RELEASED_K:                     return (key_state_t){ VK_K, false };
    case KEY_RELEASED_L:                     return (key_state_t){ VK_L, false };
    case KEY_RELEASED_SEMICOLON:             return (key_state_t){ VK_SEMICOLON, false };
    case KEY_RELEASED_SINGLE_QUOTE:          return (key_state_t){ VK_SINGLE_QUOTE, false };
    case KEY_RELEASED_BACK_TICK:             return (key_state_t){ VK_BACK_TICK, false };
    case KEY_RELEASED_LEFT_SHIFT:            return (key_state_t){ VK_LEFT_SHIFT, false };
    case KEY_RELEASED_BACK_SLASH:            return (key_state_t){ VK_BACK_SLASH, false };
    case KEY_RELEASED_Z:                     return (key_state_t){ VK_Z, false };
    case KEY_RELEASED_X:                     return (key_state_t){ VK_X, false };
    case KEY_RELEASED_C:                     return (key_state_t){ VK_C, false };
    case KEY_RELEASED_V:                     return (key_state_t){ VK_V, false };
    case KEY_RELEASED_B:                     return (key_state_t){ VK_B, false };
    case KEY_RELEASED_N:                     return (key_state_t){ VK_N, false };
    case KEY_RELEASED_M:                     return (key_state_t){ VK_M, false };
    case KEY_RELEASED_COMMA:                 return (key_state_t){ VK_COMMA, false };
    case KEY_RELEASED_PERIOD:                return (key_state_t){ VK_PERIOD, false };
    case KEY_RELEASED_FORWARD_SLASH:         return (key_state_t){ VK_FORWARD_SLASH, false };
    case KEY_RELEASED_RIGHT_SHIFT:           return (key_state_t){ VK_RIGHT_SHIFT, false };
    case KEY_RELEASED_KEYPAD_ASTERISK:       return (key_state_t){ VK_KEYPAD_ASTERISK, false };
    case KEY_RELEASED_LEFT_ALT:              return (key_state_t){ VK_LEFT_ALT, false };
    case KEY_RELEASED_SPACE:                 return (key_state_t){ VK_SPACE, false };
    case KEY_RELEASED_CAPS_LOCK:             return (key_state_t){ VK_CAPS_LOCK, false };
    case KEY_RELEASED_F1:                    return (key_state_t){ VK_F1, false };
    case KEY_RELEASED_F2:                    return (key_state_t){ VK_F2, false };
    case KEY_RELEASED_F3:                    return (key_state_t){ VK_F3, false };
    case KEY_RELEASED_F4:                    return (key_state_t){ VK_F4, false };
    case KEY_RELEASED_F5:                    return (key_state_t){ VK_F5, false };
    case KEY_RELEASED_F6:                    return (key_state_t){ VK_F6, false };
    case KEY_RELEASED_F7:                    return (key_state_t){ VK_F7, false };
    case KEY_RELEASED_F8:                    return (key_state_t){ VK_F8, false };
    case KEY_RELEASED_F9:                    return (key_state_t){ VK_F9, false };
    case KEY_RELEASED_F10:                   return (key_state_t){ VK_F10, false };
    case KEY_RELEASED_NUMBER_LOCK:           return (key_state_t){ VK_NUMBER_LOCK, false };
    case KEY_RELEASED_SCROLL_LOCK:           return (key_state_t){ VK_SCROLL_LOCK, false };
    case KEY_RELEASED_KEYPAD_7:              return (key_state_t){ VK_KEYPAD_7, false };
    case KEY_RELEASED_KEYPAD_8:              return (key_state_t){ VK_KEYPAD_8, false };
    case KEY_RELEASED_KEYPAD_9:              return (key_state_t){ VK_KEYPAD_9, false };
    case KEY_RELEASED_KEYPAD_MINUS:          return (key_state_t){ VK_KEYPAD_MINUS, false };
    case KEY_RELEASED_KEYPAD_4:              return (key_state_t){ VK_KEYPAD_4, false };
    case KEY_RELEASED_KEYPAD_5:              return (key_state_t){ VK_KEYPAD_5, false };
    case KEY_RELEASED_KEYPAD_6:              return (key_state_t){ VK_KEYPAD_6, false };
    case KEY_RELEASED_KEYPAD_PLUS:           return (key_state_t){ VK_KEYPAD_PLUS, false };
    case KEY_RELEASED_KEYPAD_1:              return (key_state_t){ VK_KEYPAD_1, false };
    case KEY_RELEASED_KEYPAD_2:              return (key_state_t){ VK_KEYPAD_2, false };
    case KEY_RELEASED_KEYPAD_3:              return (key_state_t){ VK_KEYPAD_3, false };
    case KEY_RELEASED_KEYPAD_0:              return (key_state_t){ VK_KEYPAD_0, false };
    case KEY_RELEASED_KEYPAD_PERIOD:         return (key_state_t){ VK_KEYPAD_PERIOD, false };
    case KEY_RELEASED_F11:                   return (key_state_t){ VK_F11, false };
    case KEY_RELEASED_F12:                   return (key_state_t){ VK_F12, false };

    case KEY_PRESSED_KEYPAD_ENTER:           return (key_state_t){ VK_KEYPAD_ENTER, true };
    case KEY_PRESSED_RIGHT_CTRL:             return (key_state_t){ VK_RIGHT_CTRL, true };
    case KEY_PRESSED_KEYPAD_FORWARD_SLASH:   return (key_state_t){ VK_KEYPAD_FORWARD_SLASH, true };
    case KEY_PRESSED_RIGHT_ALT:              return (key_state_t){ VK_RIGHT_ALT, true };
    case KEY_PRESSED_HOME:                   return (key_state_t){ VK_HOME, true };
    case KEY_PRESSED_ARROW_UP:               return (key_state_t){ VK_ARROW_UP, true };
    case KEY_PRESSED_PAGE_UP:                return (key_state_t){ VK_PAGE_UP, true };
    case KEY_PRESSED_ARROW_LEFT:             return (key_state_t){ VK_ARROW_LEFT, true };
    case KEY_PRESSED_ARROW_RIGHT:            return (key_state_t){ VK_ARROW_RIGHT, true };
    case KEY_PRESSED_END:                    return (key_state_t){ VK_END, true };
    case KEY_PRESSED_ARROW_DOWN:             return (key_state_t){ VK_ARROW_DOWN, true };
    case KEY_PRESSED_PAGE_DOWN:              return (key_state_t){ VK_PAGE_DOWN, true };
    case KEY_PRESSED_INSERT:                 return (key_state_t){ VK_INSERT, true };
    case KEY_PRESSED_DELETE:                 return (key_state_t){ VK_DELETE, true };
    case KEY_PRESSED_LEFT_GUI:               return (key_state_t){ VK_LEFT_GUI, true };
    case KEY_PRESSED_RIGHT_GUI:              return (key_state_t){ VK_RIGHT_GUI, true };
    case KEY_PRESSED_MENU:                   return (key_state_t){ VK_MENU, true };

    case KEY_RELEASED_KEYPAD_ENTER:          return (key_state_t){ VK_KEYPAD_ENTER, false };
    case KEY_RELEASED_RIGHT_CTRL:            return (key_state_t){ VK_RIGHT_CTRL, false };
    case KEY_RELEASED_KEYPAD_FORWARD_SLASH:  return (key_state_t){ VK_KEYPAD_FORWARD_SLASH, false };
    case KEY_RELEASED_RIGHT_ALT:             return (key_state_t){ VK_RIGHT_ALT, false };
    case KEY_RELEASED_HOME:                  return (key_state_t){ VK_HOME, false };
    case KEY_RELEASED_ARROW_UP:              return (key_state_t){ VK_ARROW_UP, false };
    case KEY_RELEASED_PAGE_UP:               return (key_state_t){ VK_PAGE_UP, false };
    case KEY_RELEASED_ARROW_LEFT:            return (key_state_t){ VK_ARROW_LEFT, false };
    case KEY_RELEASED_ARROW_RIGHT:           return (key_state_t){ VK_ARROW_RIGHT, false };
    case KEY_RELEASED_END:                   return (key_state_t){ VK_END, false };
    case KEY_RELEASED_ARROW_DOWN:            return (key_state_t){ VK_ARROW_DOWN, false };
    case KEY_RELEASED_PAGE_DOWN:             return (key_state_t){ VK_PAGE_DOWN, false };
    case KEY_RELEASED_INSERT:                return (key_state_t){ VK_INSERT, false };
    case KEY_RELEASED_DELETE:                return (key_state_t){ VK_DELETE, false };
    case KEY_RELEASED_LEFT_GUI:              return (key_state_t){ VK_LEFT_GUI, false };
    case KEY_RELEASED_RIGHT_GUI:             return (key_state_t){ VK_RIGHT_GUI, false };
    case KEY_RELEASED_MENU:                  return (key_state_t){ VK_MENU, false };

    default:
        return (key_state_t){ -1, 0 };
    }
}

uint8_t virtual_key_apply_modifiers(uint8_t vk, keyboard_state_t keyboardState)
{
    if(keyboardState.lShiftDown || keyboardState.rShiftDown)
        switch (vk)
        {
        case VK_BACK_TICK: vk = VK_TILDE; break;
        case VK_1: vk = VK_EXCLAMATION_MARK; break;
        case VK_2: vk = VK_AT; break;
        case VK_3: vk = VK_NUMBER_SIGN; break;
        case VK_4: vk = VK_DOLLAR; break;
        case VK_5: vk = VK_PERCENT; break;
        case VK_6: vk = VK_CIRCUMFLEX; break;
        case VK_7: vk = VK_AND; break;
        case VK_8: vk = VK_ASTERISK; break;
        case VK_9: vk = VK_OPEN_PARENTHESIS; break;
        case VK_0: vk = VK_CLOSE_PARENTHESIS; break;
        case VK_LESS: vk = VK_UNDER_SCORE; break;
        case VK_EQUALS: vk = VK_PLUS; break;
        
        case VK_OPEN_SQUARE_BRACKET: vk = VK_OPEN_CURLY_BRACKET; break;
        case VK_CLOSE_SQUARE_BRACKET: vk = VK_CLOSE_CURLY_BRACKET; break;
        case VK_SEMICOLON: vk = VK_COLON; break;
        case VK_SINGLE_QUOTE: vk = VK_DOUBLE_QUOTE; break;
        case VK_COMMA: vk = VK_LESS; break;
        case VK_PERIOD: vk = VK_GREATER; break;
        case VK_FORWARD_SLASH: vk = VK_QUESTION_MARK; break;
        
        default:
            break;
        }

    return vk;
}