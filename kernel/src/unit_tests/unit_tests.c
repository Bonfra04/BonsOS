#include "unit_test.h"

#include <stdio.h>
#include <string.h>

int tty_printf(const char *format, ...);

extern int test_string();

int execute_tests()
{
    if(!test_string())
    {
        tty_printf("<string.h> test went wrong.\n");
        return 0;
    }

    return 1;
    /*
    FILE* pFile = fopen("a:/test.txt", "r");
    char buff[530];
    memset(buff, 0, 530);
    fread(buff, sizeof(char), 530, pFile);
    tty_printf("%s", buff);
    fclose(pFile);
    */
}