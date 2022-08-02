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

static const kb_layout_t* current_layout;

static keyboard_state_t kb_state;
static bool ph_keystates[UINT16_MAX];
static bool vt_keystates[UINT16_MAX];

static bool is_multibyte;

static tsqueue_t key_queue;

static bool mods_match(uint8_t mods)
{
    if(mods & VK_MOD_IGNORE)
        return true;
    
    bool is_shift = (ph_keystates[KEY_LEFT_SHIFT] | ph_keystates[KEY_RIGHT_SHIFT]);
    bool is_maiusc = kb_state.capslock ^ is_shift;
    bool is_alt = ph_keystates[KEY_LEFT_ALT] && !ph_keystates[KEY_RIGHT_ALT];
    bool is_ctrl = ph_keystates[KEY_LEFT_CONTROL] && !ph_keystates[KEY_RIGHT_CONTROL];

    if(is_maiusc)
    {
        if(!(mods & VK_MOD_SHIFT) && !(mods & VK_MOD_MAIUSC))
            return false;
    }

    if(is_alt && !(mods & VK_MOD_ALT))
        return false;

    if(is_ctrl && !(mods & VK_MOD_CTRL))
        return false;

    return true;   
}

static void keyboard_isr(const interrupt_context_t* context)
{
    (void)context;

    uint8_t scancode = inportb(0x60);
    bool is_pressed = !(scancode & 0x80);

    if(scancode != 0xE0)
    {
        scancode &= ~0x80;
        uint16_t wide_scancode = is_multibyte * (0xE0 << 8) | scancode;

        const kb_layout_entry_t* entry = &(*current_layout)[wide_scancode];

        if(entry->physical == UINT16_MAX)
            kernel_panic("Unrecognized scancode: [0x%X]", wide_scancode);

        for(uint8_t i = 0; i < 4; i++)
        {
            const vk_entry_t* alt_entry = &entry->altered[i];
            if(alt_entry->vk == UINT16_MAX)
                break;

            if(mods_match(alt_entry->mods))
            {
                vt_keystates[alt_entry->vk] = is_pressed;
                ph_keystates[entry->physical] = is_pressed;

                switch (entry->physical)
                {
                case KEY_CAPS_LOCK:
                    if(is_pressed)
                        kb_state.capslock = !kb_state.capslock;
                    break;
                }

                keyevent_t event = { 
                    .is_pressed = is_pressed,
                    .vt_keycode = alt_entry->vk,
                    .ph_keycode = entry->physical
                };
                tsqueue_enqueue(&key_queue, (void*)event.value);

                break;
            }
        }
    }

    is_multibyte = scancode == 0xE0;

    ioapic_eoi();
}

void keyboard_init()
{
    memset(&kb_state, 0, sizeof(keyboard_state_t));
    memset(ph_keystates, 0, sizeof(ph_keystates));
    memset(vt_keystates, 0, sizeof(vt_keystates));

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

bool keyboard_get_vkey(uint16_t key)
{
    return vt_keystates[key];
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

void keyboard_flush()
{
    tsqueue_flush(&key_queue);
}
