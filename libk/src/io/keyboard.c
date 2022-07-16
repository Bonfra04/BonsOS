#include <io/keyboard.h>
#include <interrupts/isr_dispatcher.h>
#include <interrupts/ioapic.h>
#include <panic.h>
#include <io/ports.h>

#include <string.h>

#define KB_IRQ 1
#define KB_ISR (IRQ_OFFSET + KB_IRQ)

typedef struct kb_state
{
    bool capslock;
} keyboard_state_t;

static kb_layout_t current_layout;

static keyboard_state_t kb_state;
static bool keystates[UINT16_MAX];

static bool is_multibyte;

static void keyboard_isr(const interrupt_context_t* context)
{
    (void)context;

    uint8_t scancode = inportb(0x60);

    if(scancode != 0xE0)
    {
        uint16_t wide_scancode = is_multibyte * (0xE0 << 8) | scancode;
        key_state_t keystate = current_layout[wide_scancode];

        if(keystate.keycode == UINT16_MAX)
            kernel_panic("Unrecognized scancode: [0x%X]", wide_scancode);

        keystates[keystate.keycode] = keystate.state;

        switch (keystate.keycode)
        {
        case KEY_CAPS_LOCK:
            if(keystate.state)
                kb_state.capslock = !kb_state.capslock;
            break;
        }
    }

    is_multibyte = scancode == 0xE0;

    ioapic_eoi();
}

void keyboard_init()
{
    memset(&kb_state, 0, sizeof(keyboard_state_t));
    memset(keystates, 0, sizeof(keystates));

    is_multibyte = false;

    keyboard_layout_set(kb_layout_en_us);

    isr_set(KB_ISR, keyboard_isr);
    ioapic_unmask(KB_IRQ);
}

bool keyboard_get_key(uint16_t key)
{
    return keystates[key];
}

void keyboard_layout_set(const kb_layout_t layout)
{
    memcpy(current_layout, layout, sizeof(kb_layout_t));
}