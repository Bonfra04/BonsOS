#include <gui.h>

int main()
{
    window_t window = create_window("taskbar", 100, 100, 20, 20, WND_NO_DECORATIONS);

    for(int x = 0; x <= 100; x++)
        for(int y = 00; y <= 100; y++)
            ((uint32_t*)window.framebuffer)[y * 100 + x] = 0xFFFF0000;
    
    while(1)
        asm("pause");
}