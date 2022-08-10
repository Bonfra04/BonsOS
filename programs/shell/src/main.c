#include <stdio.h>
#include <string.h>

#include <syscalls.h>

int main()
{
    while(1)
    {
        char wd[257];
        sys_getcwd(wd, 256);
        strcat(wd, "/");

        char command[256];
        fputs(wd, stdout);
        fputs(" $> ", stdout);
        gets(command);

        if(strncmp(command, "cd", 2) == 0)
            sys_setcwd(command + 3);
        else if(strncmp(command, "echo ", 5) == 0)
            puts(command + 5);
        else if(strncmp(command, "exit", 4) == 0)
            break;
        else
            puts("Unknown command");
    }

    return 0;
}