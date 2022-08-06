#include <stdio.h>
#include <string.h>

int main()
{
    while(1)
    {
        char command[20];
        fputs("$> ", stdout);
        gets(command);

        if(strncmp(command, "echo ", 5) == 0)
            puts(command + 5);
        else if(strncmp(command, "exit", 4) == 0)
            break;
        else
            puts("Unknown command");
    }

    return 0;
}