#include <stdio.h>
#include <string.h>

int main()
{
    char pswd[512];
    printf("Insert password: ");
    scanf("%s", pswd);
    if(strcmp("1234", pswd) != 0)
        return 1;
    
    printf("Welcome back!\n");
    return 0;
}