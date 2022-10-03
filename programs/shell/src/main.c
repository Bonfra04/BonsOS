#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <syscalls.h>

#include "readline.h"

int main()
{
    while(1)
    {
        char wd[256];
        sys_getcwd(wd, 256);

        fputs(wd, stdout);
        fputs(" $> ", stdout);
        char* command = readline();
        if (!command)
            continue;

        if(strncmp(command, "cd", 2) == 0)
            sys_setcwd(command + 3);
        else if(strncmp(command, "echo ", 5) == 0)
            puts(command + 5);
        else if(strncmp(command, "exit", 4) == 0)
            break;
        else
            printf("Unknown command `%s`\n", command);

        free(command);
    }

    return 0;
}