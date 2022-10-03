#include <io/tty.h>
#include <io/keyboard/keyboard.h>
#include <timers/hpet.h>

#include <containers/deque.h>

#include <stdbool.h>

extern char* screen;
static tty_pos_t pos;
static size_t row_length;
static size_t col_length;

extern queue_t key_packet;
extern deque_t line_queue;

extern bool cursor_state;

static void cursor_blink_off()
{
    cursor_state = false;
    renderchar(screen[pos.y * row_length + pos.x]);
}

static void cursor_blink_on()
{
    cursor_state = true;
    tty_set_textcolor_bg(0xFFFFFF - tty_get_textcolor_bg());
    tty_set_textcolor_fg(0xFFFFFF - tty_get_textcolor_fg());

    renderchar(screen[pos.y * row_length + pos.x]);

    tty_set_textcolor_bg(0xFFFFFF - tty_get_textcolor_bg());
    tty_set_textcolor_fg(0xFFFFFF - tty_get_textcolor_fg());
}

static void cursor_blink_toggle()
{
    if(cursor_state)
        cursor_blink_off();
    else
        cursor_blink_on();
}

static void enqueue_key(keyevent_t k)
{
    if(!k.is_pressed)
        return;
    
    switch (k.keycode)
    {
    case KEY_BACKSPACE:
        queue_enqueue(&key_packet, (void*)127);
        return;
    case KEY_TAB:
        queue_enqueue(&key_packet, (void*)9);
        return;
    case KEY_ENTER:
        queue_enqueue(&key_packet, (void*)'\r');
        return;
    case KEY_ARROW_UP:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'A');
        return;
    case KEY_ARROW_DOWN:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'B');
        return;
    case KEY_ARROW_RIGHT:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'C');
        return;
    case KEY_ARROW_LEFT:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'D');
        return;

    case KEY_LEFT_SHIFT: case KEY_RIGHT_SHIFT:
    case KEY_LEFT_CONTROL: case KEY_RIGHT_CONTROL:
    case KEY_LEFT_ALT: case KEY_RIGHT_ALT:
    case KEY_CAPS_LOCK:
        return;
    }

    uint16_t vk = keyboard_translate_vk(k.keycode);
    if(vk == VK_NONE)
        return;
    char ch = vk;

    bool is_ctrl = keyboard_get_key(KEY_LEFT_CONTROL) || keyboard_get_key(KEY_RIGHT_CONTROL);
    if(isalpha(ch) && is_ctrl)
    {
        queue_enqueue(&key_packet, (void*)(uint64_t)(ch - 'a' + 1));
        return;
    }

    queue_enqueue(&key_packet, (void*)(uint64_t)ch);
}

static uint8_t read_char()
{
    uint64_t start_time = hpet_current_nanos();

    cursor_blink_on();
    while(true)
    {
        if(queue_size(&key_packet) > 0)
        {
            cursor_blink_off();
            return (uint64_t)queue_dequeue(&key_packet);
        }

        if(keyboard_has_event())
            enqueue_key(keyboard_pull());

        if(hpet_current_nanos() - start_time > 100000000 * 5)
        {
            cursor_blink_toggle();
            start_time = hpet_current_nanos();
        }
    }

}

extern void print_char(char ch, bool raw);

static void buffer_line()
{
    char ch;
    while (ch = read_char())
    {
        if(ch == 127)
        {
            print_char('\b', false);
            deque_pop_back(&line_queue);
            continue;
        }
        
        print_char(ch, true);
        if(ch == '\r')
        {
            print_char('\n', true);
            return;
        }

        deque_push_back(&line_queue, (void*)(uint64_t)ch);
    }
    
}

size_t tty_read(char* buf, size_t size, bool raw)
{
    if(raw)
    {
        for(size_t i = 0; i < size; i++)
            buf[i] = read_char();
        return size;
    }

    if(deque_size(&line_queue) == 0)
        buffer_line();

    size_t i;
    for(i = 0; i < size && deque_size(&line_queue) > 0; i++)
        buf[i] = (uint64_t)deque_pop_front(&line_queue);

    return size - i;
}
