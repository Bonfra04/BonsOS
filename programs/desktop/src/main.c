#include <stdlib.h>
#include "renderer/renderer.h"
#include <syscalls.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void update();
void render();

int main(int argc, char* argv[])
{
    renderer_init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    while(1)
    {
        renderer_clear(0xFF000000);
        update();
        render();
        swap_buffers();
    }

    return 0;
}

void update()
{
}

void render()
{
    uint64_t x, y;
    get_mouse(&x, &y);
    renderer_put_pixel(x, y, 0xFFFFFFFF);
}