#include <gui.h>
#include <syscalls.h>

static uint32_t color;

void event_handler(uint64_t wnd_id, uint8_t event_id)
{
    switch(event_id)
    {
        case EVENT_MOUSE_LEFT_DOWN:
            color = 0xFF00FF00;
            break;
        case EVENT_MOUSE_LEFT_UP:
            color = 0xFFFF0000;
            break;
        default:
            default_event_handler(wnd_id, event_id);
            break;
    }
}

int main()
{
    uint64_t width = display_width();
    uint64_t height = display_height() / 25;
    uint64_t x = 0, y = display_height() - height;
    window_t window = window_create("taskbar", width, height, x, y, WND_NO_DECORATIONS);

    color = 0xFFFFFFFF;

    do
    {
        for(int x = 0; x < width; x++)
            for(int y = 0; y < height; y++)
                ((uint32_t*)window.framebuffer)[y * width + x] = color;
    } while(cycle_events(event_handler));

    while(1)
        asm("pause");

    return 0;
}