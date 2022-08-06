#include <stdio.h>
#include <string.h>

#include <syscalls.h>

int main()
{
    fopen("tty:/0", "r"); // open stdin
    fopen("tty:/0", "w"); // open stdout
    fopen("tty:/0", "w"); // open stderr

    puts("What's your name? ");
    
    char name[10];
    gets(name); // buffer overflow goes brrrr
    
    fputs("Hello ", stdout);
    fputs(name, stdout);
    puts(".");

    sys_exec("a:/bin/shell.elf", NULL);

    return 0;
}
