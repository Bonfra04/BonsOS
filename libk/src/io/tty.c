#include <io/tty.h>

#include <graphics/screen.h>
#include <graphics/text_renderer.h>
#include <io/keyboard/keyboard.h>
#include <timers/hpet.h>

#include <containers/deque.h>
#include <containers/queue.h>

#include <ctype.h>

#define TAB_SIZE 4

static uint32_t fg_color;
static uint32_t bg_color;
static tty_pos_t pos;

static size_t row_length;
static size_t col_length;
static size_t char_width;
static size_t char_height;

static queue_t key_packet;
static deque_t line_queue;

static bool cursor_state = false;

void tty_init()
{
    fg_color = 0xFFFFFFFF;
    bg_color = 0xFF000000;
    pos.x = 0;
    pos.y = 0;

    char_width = text_renderer_charwidth(1);
    char_height = text_renderer_charheight(1);
    row_length = screen_get_width() / char_width;
    col_length = screen_get_height() / char_height;

    line_queue = deque();
    key_packet = queue();

    tty_clear();
}

uint32_t tty_get_textcolor_fg()
{
    return fg_color;
}

uint32_t tty_get_textcolor_bg()
{
    return bg_color;
}

void tty_set_textcolor(uint32_t fg, uint32_t bg)
{
    fg_color = fg;
    bg_color = bg;
}

void tty_set_textcolor_fg(uint32_t fg)
{
    fg_color = fg;
}

void tty_set_textcolor_bg(uint32_t bg)
{
    bg_color = bg;
}

tty_pos_t tty_getpos()
{
    return pos;
}

void tty_setpos(tty_pos_t tty_pos)
{
    pos = tty_pos;
}

void tty_clear()
{
    pos.x = 0;
    pos.y = 0;
    screen_clear(bg_color);
}

static void print_char(char ch)
{
    if(ch == '\n')
    {
        pos.x = row_length;
    }
    else if(ch == '\r')
    {
        pos.x = 0;
    }
    else if(ch == '\t')
    {
        if(pos.x % TAB_SIZE == 0)
            pos.x += TAB_SIZE;
        else
            pos.x += TAB_SIZE - pos.x % TAB_SIZE;
    }
    else if(ch == '\b')
    {
        if(pos.x > 0)
        {
            pos.x--;
            text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ' ', fg_color, bg_color);
        }
        else if(pos.y > 0)
        {
            pos.y--;
            pos.x = row_length - 1;
            text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ' ', fg_color, bg_color);
        }
    }
    else
    {
        text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ch, fg_color, bg_color);
        pos.x++;
    }

    if (pos.x == row_length)
    {
        pos.x = 0;
        pos.y++;
    }
}

void tty_print(const char* str)
{
    while(*str)
        print_char(*str++);
}

static void cursor_blink_off()
{
    cursor_state = false;
    text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, ' ', fg_color, bg_color);
}

static void cursor_blink_on()
{
    cursor_state = true;
    text_renderer_putchar(pos.x * char_width, pos.y * char_height, 1, '_', fg_color, bg_color);
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
    case KEY_ARROW_LEFT:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'D');
        return;
    case KEY_ARROW_RIGHT:
        queue_enqueue(&key_packet, (void*)27);
        queue_enqueue(&key_packet, (void*)'[');
        queue_enqueue(&key_packet, (void*)'C');
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

    bool is_ctrl = keyboard_get_key(VK_LEFT_CONTROL) || keyboard_get_key(VK_RIGHT_CONTROL);
    if(isalpha(ch) && is_ctrl)
    {
        queue_enqueue(&key_packet, (void*)(ch - 'a' + 1));
        return;
    }

    queue_enqueue(&key_packet, (void*)ch);
}

uint8_t tty_read_raw()
{
    uint64_t start_time = hpet_current_nanos();

    while(true)
    {
        if(keyboard_has_event())
            enqueue_key(keyboard_pull());

        if(queue_size(&key_packet) > 0)
            return (uint8_t)queue_dequeue(&key_packet);

        if(hpet_current_nanos() - start_time > 100000000 * 5)
        {
            cursor_blink_toggle();
            start_time = hpet_current_nanos();
        }
    }

}

static void buffer_line()
{
    uint64_t line_pos = 0;

    uint8_t ch;
    do
    {
        ch = tty_read_raw();
        switch (ch)
        {
        case '\r':
            ch = '\n';
            break;

        case 127:
            if(line_pos > 0)
            {
                line_pos--;
                print_char('\b');
                deque_pop_back(&line_queue);
            }
            continue;
        }
        print_char(ch);
        deque_push_back(&line_queue, (void*)ch);
        line_pos++;
    }
    while(ch != '\n');
}

size_t tty_read(char* buf, size_t size)
{
    if(deque_size(&line_queue) == 0)
        buffer_line();

    size_t i = size;
    while(i && deque_size(&line_queue) > 0)
    {
        *buf++ = (char)(uint64_t)deque_pop_front(&line_queue);
        i--;
    }

    return size - i;
    return 0;
}
