#include <gui.h>
#include <syscalls.h>
#include <rund.h>

void event_handler(uint64_t wnd_id, uint8_t event_id)
{
    switch(event_id)
    {
        default:
            default_event_handler(wnd_id, event_id);
            break;
    }
}

void render(const window_t* window)
{
    for(int x = 0; x < window->width; x++)
        for(int y = 0; y < window->height; y++)
            ((uint32_t*)window->framebuffer)[y * window->width + x] = 0xFFFFFFFF;
}

int main()
{
    size_t width = display_width();
    size_t height = display_height();// / 25;
    uint64_t x = 0, y = display_height() - height;

    rund_app_t app = {
        "Taskbar",
        x, y,
        width, height,
        Row(
			list(
				Container(NULL, 100, 100, ContainerDec{ .color = 0x00FF00 }),
				Expanded(
					Container(NULL, ContainerDec{ .color = 0x0000FF }),
					Flex(1)
				),
				Expanded(
					Container(NULL, ContainerDec{ .color = 0xFFFF00 }),
					Flex(2)
				),
				Expanded(
					Container(NULL, ContainerDec{ .color = 0xFFFFFF }),
					Flex(3)
				),
				Align(
					Container(NULL, 100, 100, ContainerDec{ .color = 0xFF006F }),
					Alignment(0.0f, 1.0f)
				),
				Container(NULL, 100, 100)
			)
		)
    };

    run_app(&app);

    while(1)
        asm("pause");

    return 0;
}