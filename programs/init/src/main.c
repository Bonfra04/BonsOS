#include <stdio.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char* argv[])
{
    printf("Welcome in BonsOS\n");
    for(int i = 0; i < argc; i++)
        printf("%s\n", argv[i]);
    return 0;
}