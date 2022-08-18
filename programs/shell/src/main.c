#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#include <line.h>

#include <syscalls.h>

int main()
{
    freopen("tty:/raw", "r", stdin);
    
    line_t* line = line_init();
    if (!line || !line->buff_left || !line->buff_right)
    {
        fputs("calloc of input buffer failed\n", stderr);
        return -1;
    }

    while(1)
    {
        char wd[257];
        sys_getcwd(wd, 256);
        strcat(wd, "/");

        fputs(wd, stdout);
        fputs(" $> ", stdout);
        char* command = line_read(line);
        if (!command)
            continue;

        if(strncmp(command, "cd", 2) == 0)
            sys_setcwd(command + 3);
        else if(strncmp(command, "echo ", 5) == 0)
            puts(command + 5);
        else if(strncmp(command, "exit", 4) == 0)
            break;
        else
            puts("Unknown command");

        free(command);
    }

    line_free(line);

    return 0;
}