#include <stdlib.h>
#include "renderer/renderer.h"
#include "renderer/tga.h"
#include <syscalls.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void update();
void render();
void init();

int main(int argc, char* argv[])
{
    renderer_init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    init();

    while(1)
    {
        renderer_clear(0xFF000000);
        update();
        render();
        swap_buffers();
    }

    return 0;
}

static tga_t cursor;

void init()
{
    tga_load(&cursor, "a:/assets/cursor.tga");
}

static uint64_t mouse_x, mouse_y;
void update()
{
    get_mouse(&mouse_x, &mouse_y);
}

void render()
{
    int bar_height = display_height() / 20;
    fill_rect(0, display_height() - bar_height, display_width(), bar_height, 0xFF33E6FF);

    draw_image(&cursor, mouse_x, mouse_y);
}
