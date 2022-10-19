#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <syscalls.h>

#include "readline.h"

static const char** parse_args(const char* line)
{
    int argc = 0;

    {
        char* line_copy = strdup(line);
        for(char* token = strtok(line_copy, " \t"); token != NULL; token = strtok(NULL, " \t"))
            argc++;
        free(line_copy);
    }

    const char** argv = malloc((argc + 1) * sizeof(char*));
    argv[argc] = NULL;

    {
        int i = 0;

        char* line_copy = strdup(line);
        for(char* token = strtok(line_copy, " \t"); token != NULL; token = strtok(NULL, " \t"))
            argv[i++] = strdup(token);
        free(line_copy);
    }

    return argv;
}

#include "builtins.h"

static void execute_command(const char** args)
{
    if(args[0] == NULL)
        return;

    for(uint64_t i = 0; i < NUM_BUILTINS; i++)
        if(strcmp(args[0], builtin_str[i]) == 0)
        {
            (*builtin_func[i])(args);
            return;
        }

    // TODO: execute command
}

int main()
{
    // TEST thing
    sys_raise_signal(2, 0xbad);

    while(1)
    {
        fputs("$> ", stdout);

        char* command = readline();
        if (!command)
            continue;

        const char** args = parse_args(command);
        if(!args)
            continue;

        execute_command(args);

        free(command);
        for(const char** arg = &args[0]; *arg != NULL; arg++)
            free((void*)*arg);
        free(args);
    }

    return EXIT_SUCCESS;
}