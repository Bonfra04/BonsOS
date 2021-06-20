#include <device/keyboard.h>
#include <interrupt/interrupt.h>
#include <panic.h>
#include <ctype.h>
#include <x86/ports.h>

#include <device/keyboard/kb_us.h>

#define IRQ_KEYBOARD        1
#define TRAP_IRQ_KEYBOARD   0x21

extern uint64_t kb_handler;

static keyboard_state_t keyboardState;
static key_state_t keyStates[0xFF];

static uint8_t lastHit;
static bool isMultiByte;

static void isr_keyboard(const interrupt_context_t* context)
{
    (void)context;

    uint8_t scancode = inportb(0x60);

    if(scancode != 0xE0)
    {
        uint16_t wideScancode = isMultiByte * (0xE0 << 8) | scancode;
        key_state_t keyState = physical_to_virtual_key(wideScancode);

        if(keyState.keycode == 0xFF)
            kenrel_panic("Unrecognized scancode: [0x%X]", wideScancode);

        keyStates[keyState.keycode] = keyState;

        if(keyState.isDown)
            lastHit = keyState.keycode;

        switch (keyState.keycode)
        {
        case VK_LEFT_SHIFT:
            keyboardState.lShiftDown = keyState.isDown;
            break;
        case VK_RIGHT_SHIFT:
            keyboardState.rShiftDown = keyState.isDown;
            break;
        case VK_LEFT_CTRL:
            keyboardState.lCtrlDown = keyState.isDown;
            break;
        case VK_RIGHT_CTRL:
            keyboardState.rCtrlDown = keyState.isDown;
            break;
        case VK_LEFT_ALT:
            keyboardState.lAltDown = keyState.isDown;
            break;
        case VK_RIGHT_ALT:
            keyboardState.rAltDown = keyState.isDown;
            break;
        case VK_CAPS_LOCK:
            if(keyState.isDown)
                keyboardState.isCapsLock = !keyboardState.isCapsLock;
            break;
        }
    }

    isMultiByte = scancode == 0xE0;

    ISR_DONE();
}

void kb_init(void)
{
    isMultiByte = false;
    lastHit = 0xFF;

    for(int i = 0; i < 0xFF; i++)
        keyStates[i] = (key_state_t){ i, false };

    keyboardState.isCapsLock = false;
    keyboardState.lAltDown = false;
    keyboardState.rAltDown = false;
    keyboardState.rCtrlDown = false;
    keyboardState.lCtrlDown = false;
    keyboardState.rShiftDown = false;
    keyboardState.lShiftDown = false;

    isr_set(TRAP_IRQ_KEYBOARD, isr_keyboard);
    irq_enable(IRQ_KEYBOARD);
}

bool kb_is_key_down(uint8_t key)
{
    return keyStates[key].isDown;
}

uint8_t kb_wait_hit(void)
{
    while(lastHit == 0xFF);
    uint8_t tmp = lastHit;
    lastHit = 0xFF;
    return tmp;
}

int kb_getch(void)
{
    uint8_t keycode = kb_wait_hit();
    int res = vk_to_char(keycode);
    if(res == 0)
        res = 0xFF + keycode;
    return res;
}

char vk_to_char(uint8_t vk)
{
    vk = virtual_key_apply_modifiers(vk, keyboardState);

    char ret;

    switch (vk)
    {
    case VK_BACK_TICK: ret = '`'; break;
    case VK_1: ret = '1'; break;
    case VK_2: ret = '2'; break;
    case VK_3: ret = '3'; break;
    case VK_4: ret = '4'; break;
    case VK_5: ret = '5'; break;
    case VK_6: ret = '6'; break;
    case VK_7: ret = '7'; break;
    case VK_8: ret = '8'; break;
    case VK_9: ret = '9'; break;
    case VK_0: ret = '0'; break;
    case VK_MINUS: ret = '-'; break;
    case VK_EQUALS: ret = '='; break;
    case VK_BACKSPACE: ret = '\b'; break;

    case VK_TAB: ret = '\t'; break;
    case VK_Q: ret = 'q'; break;
    case VK_W: ret = 'w'; break;
    case VK_E: ret = 'e'; break;
    case VK_R: ret = 'r'; break;
    case VK_T: ret = 't'; break;
    case VK_Y: ret = 'y'; break;
    case VK_U: ret = 'u'; break;
    case VK_I: ret = 'i'; break;
    case VK_O: ret = 'o'; break;
    case VK_P: ret = 'p'; break;
    case VK_OPEN_SQUARE_BRACKET: ret = '['; break;
    case VK_CLOSE_SQUARE_BRACKET: ret = ']'; break;
    case VK_BACK_SLASH: ret = '\\'; break;

    case VK_A: ret = 'a'; break;
    case VK_S: ret = 's'; break;
    case VK_D: ret = 'd'; break;
    case VK_F: ret = 'f'; break;
    case VK_G: ret = 'g'; break;
    case VK_H: ret = 'h'; break;
    case VK_J: ret = 'j'; break;
    case VK_K: ret = 'k'; break;
    case VK_L: ret = 'l'; break;
    case VK_SEMICOLON: ret = ';'; break;
    case VK_SINGLE_QUOTE: ret = '\''; break;
    case VK_ENTER: ret = '\n'; break;

    case VK_Z: ret = 'z'; break;
    case VK_X: ret = 'x'; break;
    case VK_C: ret = 'c'; break;
    case VK_V: ret = 'v'; break;
    case VK_B: ret = 'b'; break;
    case VK_N: ret = 'n'; break;
    case VK_M: ret = 'm'; break;
    case VK_COMMA: ret = ','; break;
    case VK_PERIOD: ret = '.'; break;
    case VK_FORWARD_SLASH: ret = '/'; break;

    case VK_SPACE: ret = ' '; break;

    case VK_KEYPAD_FORWARD_SLASH: ret = '/'; break;
    case VK_KEYPAD_ASTERISK: ret = '*'; break;
    case VK_KEYPAD_MINUS: ret = '-'; break;
    case VK_KEYPAD_PLUS: ret = '+'; break;
    case VK_KEYPAD_PERIOD: ret = '.'; break;
    case VK_KEYPAD_7: ret = '7'; break;
    case VK_KEYPAD_8: ret = '8'; break;
    case VK_KEYPAD_9: ret = '9'; break;
    case VK_KEYPAD_4: ret = '4'; break;
    case VK_KEYPAD_5: ret = '5'; break;
    case VK_KEYPAD_6: ret = '6'; break;
    case VK_KEYPAD_1: ret = '1'; break;
    case VK_KEYPAD_2: ret = '2'; break;
    case VK_KEYPAD_3: ret = '3'; break;
    case VK_KEYPAD_0: ret = '0'; break;
    case VK_KEYPAD_ENTER: ret = '\n'; break;

    case VK_TILDE: ret = '~'; break;
    case VK_EXCLAMATION_MARK: ret = '!'; break;
    case VK_AT: ret = '@'; break;
    case VK_NUMBER_SIGN: ret = '#'; break;
    case VK_DOLLAR: ret = '$'; break;
    case VK_PERCENT: ret = '%'; break;
    case VK_CIRCUMFLEX: ret = '^'; break;
    case VK_AND: ret = '&'; break;
    case VK_ASTERISK: ret = '*'; break;
    case VK_OPEN_PARENTHESIS: ret = '('; break;
    case VK_CLOSE_PARENTHESIS: ret = ')'; break;
    case VK_UNDER_SCORE: ret = '_'; break;
    case VK_PLUS: ret = '+'; break;
    case VK_OPEN_CURLY_BRACKET: ret = '{'; break;
    case VK_CLOSE_CURLY_BRACKET: ret = '}'; break;
    case VK_PIPE: ret = '|'; break;
    case VK_COLON: ret = ':'; break;
    case VK_DOUBLE_QUOTE: ret = '"'; break;
    case VK_LESS: ret = '<'; break;
    case VK_GREATER: ret = '>'; break;
    case VK_QUESTION_MARK: ret = '?'; break;

    default:
        ret = 0; break;
    }

    if(keyboardState.isCapsLock && !(keyboardState.lShiftDown || keyboardState.rShiftDown))
        ret = toupper(ret);
    if(!keyboardState.isCapsLock && (keyboardState.lShiftDown || keyboardState.rShiftDown))
        ret = toupper(ret);

    return ret;
}