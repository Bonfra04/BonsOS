#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <syscalls.h>

typedef struct system_info
{
    void* framebuffer;
    size_t screen_width;
    size_t sreen_height;
    size_t screen_pitch;
} system_info_t;

uint64_t run_windows_manager(system_info_t* system_info)
{
    char fb[16], sw[16], sh[16], sp[16];
    ulltoa(system_info->framebuffer, fb, 16);
    ulltoa(system_info->screen_width, sw, 16);
    ulltoa(system_info->sreen_height, sh, 16);
    ulltoa(system_info->screen_pitch, sp, 16);
    char* argv[] = { fb, sw, sh, sp };

    printf("Launching windows manager\n");
    return run_executable("a:/bin/wndmng.elf", 4, argv, ELF);
}

int main(int argc, char* argv[])
{
    system_info_t system_info;
    system_info.framebuffer = (void*)strtoull(argv[0], NULL, 16);
    system_info.screen_width = strtoull(argv[1], NULL, 16);
    system_info.sreen_height = strtoull(argv[2], NULL, 16);
    system_info.screen_pitch = strtoull(argv[3], NULL, 16);

    uint64_t windows_manager_pid = run_windows_manager(&system_info);

    printf("Intialization sequence terminated\n");
    while(1)
        asm("pause"); // my code sucks
    return 0;
}
