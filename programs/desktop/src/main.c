#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "renderer/renderer.h"
#include <stdbool.h>

int main(int argc, char* argv[])
{
    renderer_init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    return 0;
}