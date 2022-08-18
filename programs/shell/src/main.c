#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shell.h>

escape_key_t read_escape()
{
    if (getchar() != '[')
        return ESCAPE_NONE;

    switch (getchar())
    {
    case 'A': // ARROW UP
        return ARROW_UP;

    case 'B': // ARROW DOWN
        return ARROW_DOWN;

    case 'C': // ARROW RIGHT
        return ARROW_RIGHT;

    case 'D': // ARROW LEFT
        return ARROW_LEFT;
    default:
        return ESCAPE_NONE;
    }

    return ESCAPE_NONE;
}

int main()
{
    freopen("tty:/raw", "r", stdin);
    
    shell_t* shell = shell_init();

    if (!shell || !shell->buff_left || !shell->buff_right)
    {
        fputs("calloc of input buffer failed\n", stderr);
        return -1;
    }

    while(1)
    {
        int input_char = getchar();

        if (input_char == EOF)
            continue;

        switch (input_char)
        {
        case '\r':
            shell_submit(shell);
            break;
        case 127:
            shell_delete(shell);
            break;
        case 27:
            shell_handle_escape(shell, read_escape());
            break;
        default:
            shell_print(shell, input_char);
        }
    }

    return 0;
}