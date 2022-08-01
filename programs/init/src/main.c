#include <string.h>
#include <stdio.h>

int main()
{
    fopen("tty:/0", "r"); // open stdin
    fopen("tty:/0", "w"); // open stdout
    fopen("tty:/0", "w"); // open stderr

    puts("What's your name? ");
    
    char name[5];
    memset(name, '\0', 5);
    fread(name, 4, 1, stdin);
    
    fputs("Hello ", stdout);
    fputs(name, stdout);
    puts(".");

    return 0;
}
