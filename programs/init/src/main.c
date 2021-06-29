#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct system_info
{
    void* framebuffer;
    size_t screen_width;
    size_t sreen_height;
} system_info_t;

int main(int argc, char* argv[])
{
    printf("Welcome in BonsOS\n");

    system_info_t system_info;
    system_info.framebuffer = (void*)strtoull(argv[0], 0, 16);
    system_info.screen_width = strtoull(argv[1], 0, 16);
    system_info.sreen_height = strtoull(argv[2], 0, 16);

    printf("%X, %X, %X", system_info.framebuffer, system_info.screen_width, system_info.sreen_height);
    return 0;
}