#include <io/keyboard/keyboard.h>
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

static const kb_layout_t* current_layout;

static keyboard_state_t kb_state;
static bool ph_keystates[UINT16_MAX];

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

        uint16_t keycode = current_layout->physical[wide_scancode];

        if(keycode == UINT16_MAX)
            kernel_panic("Unrecognized scancode: [0x%X]", wide_scancode);

        ph_keystates[keycode] = is_pressed;
        keyevent_t event = { .is_pressed = is_pressed, .keycode = keycode };
        tsqueue_enqueue(&key_queue, (void*)event.value);

        switch (keycode)
        {
        case KEY_CAPS_LOCK:
            if(is_pressed)
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
    memset(ph_keystates, 0, sizeof(ph_keystates));

    is_multibyte = false;

    keyboard_layout_set(&kb_layout_en_us);

    isr_set(KB_ISR, keyboard_isr);
    ioapic_unmask(KB_IRQ);

    key_queue = tsqueue();
}

bool keyboard_get_key(uint16_t key)
{
    return ph_keystates[key];
}

void keyboard_layout_set(const kb_layout_t* layout)
{
    current_layout = layout;
}

keyevent_t keyboard_pull()
{
    tsqueue_wait(&key_queue);
    keyevent_t event;
    event.value = (uint64_t)tsqueue_dequeue(&key_queue);
    return event;
}

bool keyboard_has_event()
{
    return tsqueue_size(&key_queue) > 0;
}

void keyboard_flush()
{
    tsqueue_flush(&key_queue);
}

uint16_t keyboard_translate_vk(uint16_t physical_key)
{
    // TODO: capslock, altgr
    
    if(ph_keystates[KEY_LEFT_SHIFT] | ph_keystates[KEY_RIGHT_SHIFT])
        return current_layout->virtual[physical_key].vk_shift;
    else
        return current_layout->virtual[physical_key].vk_normal;
}
