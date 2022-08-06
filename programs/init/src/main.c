#include <stdio.h>
#include <string.h>

#include <syscalls.h>

int main()
{
    // stream inherithed from childs
    fopen("tty:/0", "r"); // open stdin
    fopen("tty:/0", "w"); // open stdout
    fopen("tty:/0", "w"); // open stderr

    sys_exec("a:/bin/shell.elf", NULL);

    return 0;
}
