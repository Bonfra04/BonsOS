#include <gui.h>

int main()
{
    window_t window = create_window(0, 100, 100, 0, display_height(), WND_NO_DECORATIONS);

    while(1)
        asm("pause");
}