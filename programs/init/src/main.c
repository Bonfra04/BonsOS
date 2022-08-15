#include <stdio.h>
#include <string.h>

#include <syscalls.h>

int main()
{
    // stream inherithed from childs
    fopen("tty:/cooked", "r"); // open stdin
    fopen("tty:/cooked", "w"); // open stdout
    fopen("tty:/cooked", "w"); // open stderr

    sys_exec("a:/bin/shell.elf", NULL, NULL);

    return 0;
}
