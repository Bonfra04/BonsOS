#include <stdlib.h>
#include "renderer/renderer.h"
#include <syscalls.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    renderer_init((void*)strtoull(argv[0], 0, 16), strtoull(argv[1], 0, 16), strtoull(argv[2], 0, 16), strtoull(argv[3], 0, 16));

    {
        msg_t msg;
        memset(msg.data, 1, sizeof(msg_t));
        msg.id = 30;

        msg_send(2, &msg);
    }
    msg_t msg;
    msg_fetch(&msg);
    printf("%d\n", msg.id);
    for(int i = 0; i < 31; i++)
        printf("%d", msg.data[i]);

    return 0;
}