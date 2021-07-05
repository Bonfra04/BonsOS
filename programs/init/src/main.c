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

uint64_t run_desktop_manager(system_info_t* system_info)
{
    char fb[16], sw[16], sh[16], sp[16];
    ulltoa(system_info->framebuffer, fb, 16);
    ulltoa(system_info->screen_width, sw, 16);
    ulltoa(system_info->sreen_height, sh, 16);
    ulltoa(system_info->screen_pitch, sp, 16);
    char* argv[] = { fb, sw, sh, sp };

    return run_executable("a:/bin/desktop.elf", 4, argv, ELF);
}

int main(int argc, char* argv[])
{
    system_info_t system_info;
    system_info.framebuffer = (void*)strtoull(argv[0], NULL, 16);
    system_info.screen_width = strtoull(argv[1], NULL, 16);
    system_info.sreen_height = strtoull(argv[2], NULL, 16);
    system_info.screen_pitch = strtoull(argv[3], NULL, 16);

    uint64_t desktop_manager_pid = run_desktop_manager(&system_info);

    // tmp
    while(1);
    return 0;
}
