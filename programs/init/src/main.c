#include <stdio.h>
#include <string.h>

int main()
{
    fopen("tty:/0", "r"); // open stdin
    fopen("tty:/0", "w"); // open stdout
    fopen("tty:/0", "w"); // open stderr

    // for(;;)
    // {
    //     int c = getchar();
    //     putchar(c);
    //     putchar('\n');
    // }

    puts("What's your name? ");
    
    char name[5];
    fgets(name, 5, stdin);
    
    fputs("Hello ", stdout);
    fputs(name, stdout);
    puts(".");

    return 0;
}
