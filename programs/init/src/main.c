#include <string.h>
#include <stdio.h>

int main()
{
    fopen("tty:/0", "r"); // open stdin
    fopen("tty:/0", "w"); // open stdout
    fopen("tty:/0", "w"); // open stderr

    fwrite("What's your name? ", 18, 1, stdout);
    
    char name[5];
    memset(name, '\0', 5);
    fread(name, 4, 1, stdin);
    
    fwrite("Hello ", 6, 1, stdout);
    fwrite(name, strlen(name), 1, stdout);
    fwrite(".", 1, 1, stdout);

    return 0;
}
