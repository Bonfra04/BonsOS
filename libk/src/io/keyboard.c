#include <io/keyboard.h>
#include <interrupts/isr_dispatcher.h>
#include <interrupts/ioapic.h>
#include <panic.h>
#include <io/ports.h>

#include <containers/tsqueue.h>

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

static tsqueue_t key_queue;

static void keyboard_isr(const interrupt_context_t* context)
{
    (void)context;

    uint8_t scancode = inportb(0x60);
    bool is_pressed = !(scancode & 0x80);

    if(scancode != 0xE0)
    {
        scancode &= ~0x80;
        uint16_t wide_scancode = is_multibyte * (0xE0 << 8) | scancode;
        uint16_t keycode = current_layout[wide_scancode];

        if(keycode == UINT16_MAX)
            kernel_panic("Unrecognized scancode: [0x%X]", wide_scancode);

        keystates[keycode] = is_pressed;

        switch (keycode)
        {
        case KEY_CAPS_LOCK:
            if(is_pressed)
                kb_state.capslock = !kb_state.capslock;
            break;
        }

        keyevent_t event = {.keycode = keycode, .is_pressed = is_pressed};
        tsqueue_enqueue(&key_queue, (void*)event.value);
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

    key_queue = tsqueue();
}

bool keyboard_get_key(uint16_t key)
{
    return keystates[key];
}

void keyboard_layout_set(const kb_layout_t layout)
{
    memcpy(current_layout, layout, sizeof(kb_layout_t));
}

keyevent_t keyboard_pull()
{
    tsqueue_wait(&key_queue);
    keyevent_t event;
    event.value = (uint64_t)tsqueue_dequeue(&key_queue);
    return event;
}

void keyboard_flush()
{
    tsqueue_flush(&key_queue);
}
