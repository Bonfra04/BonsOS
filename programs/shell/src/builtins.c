#include "builtins.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdnoreturn.h>

static noreturn int builtin_exit(const char** args)
{
    (void)args;
    exit(EXIT_SUCCESS);
}

static int builtin_cd(const char** args)
{
    (void)args;
    if(args[1] == NULL)
    {
        fprintf(stderr, "cd: expected argument to \"cd\"\n");
        return EXIT_FAILURE;
    }

    printf("cd to %s\n", args[1]);
    return EXIT_SUCCESS;
}

const char* builtin_str[NUM_BUILTINS] = {
    "exit",
    "cd",    
};

int (*builtin_func[NUM_BUILTINS])(const char** args) = {
    &builtin_exit,
    &builtin_cd,
};
