#include <string.h>
#include <stdio.h>
// This tests are based on this [https://www.cplusplus.com/reference/cstring/]

int test_memcpy()
{
    char mem0[] = "this is a test stirng";
    const size_t len = sizeof(mem0) / sizeof(mem0[0]);
    char mem1[len];
    for(int i = 0; i < len; i++)
        mem1[i] = 1;

    char* dest = (char*)memcpy(mem1, mem0, len);

    if(dest != mem1)
        return 0;
    for(int i = 0; i < len; i++)
        if(mem1[i] != mem0[i])
            return 0;
    return 1;
}

int test_memmove()
{
    char mem0[] = "this is a test stirng - other stuffs";

    char* dest = (char*)memmove(mem0 + 21, mem0, 15);

    if(dest != mem0 + 21)
        return 0;
    for(int i = 0; i < 15; i++)
        if(mem0[i] != mem0[i + 21])
            return 0;
    return 1;
}

int test_strcpy()
{
    char mem0[] = "this is a test string";
    const size_t len = sizeof(mem0) / sizeof(mem0[0]);
    char mem1[len];
    for(int i = 0; i < len; i++)
        mem1[i] = 1;

    char* dest = (char*)strcpy(mem1, mem0);

    if(dest != mem1)
        return 0;
    for(int i = 0; i < len; i++)
        if(mem0[i] != mem1[i])
            return 0;
    return 1;
}

int test_strncpy()
{
    {
        char mem0[] = "this is a test string";
        const size_t len = sizeof(mem0) / sizeof(mem0[0]);
        char mem1[len];
        for(int i = 0; i < len; i++)
            mem1[i] = 1;

        char* dest = (char*)strncpy(mem1, mem0, len - 3);

        if(dest != mem1)
            return 0;
        for(int i = 0; i < len; i++)
            if(i < len - 3)
            {
                if(mem0[i] != mem1[i])
                    return 0;
            }
            else if(mem1[i] != 1)
                return 0;
    }

    {
        char mem0[] = "this is a test string";
        const size_t len = sizeof(mem0) / sizeof(mem0[0]);
        char mem1[len + 3];
        for(int i = 0; i < len + 3; i++)
            mem1[i] = 1;

        char* dest = (char*)strncpy(mem1, mem0, len + 3);

        if(dest != mem1)
            return 0;
        for(int i = 0; i < len + 3; i++)
            if(i < len)
            {
                if(mem0[i] != mem1[i])
                    return 0;
            }
            else if(mem1[i] != 0)
                return 0;
    }

    return 1;
}

int test_string()
{
    if(!test_memcpy())
    {
        printf("'memcpy' test went wrong.\n");
        return 0;
    }
    if(!test_memmove())
    {
        printf("'memmove' test went wrong.\n");
        return 0;
    }
    if(!test_strcpy())
    {
        printf("'strcpy' test went wrong.\n");
        return 0;
    }
    if(!test_strncpy())
    {
        printf("'strncpy' test went wrong.\n");
        return 0;
    }

    return 1;
}