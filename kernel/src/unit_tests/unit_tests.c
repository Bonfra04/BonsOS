#include "unit_test.h"

#include <stdio.h>
#include <string.h>

extern int test_string();

int execute_tests()
{
    if(!test_string())
    {
        printf("<string.h> test went wrong.\n");
        return 0;
    }

    return 1;
}