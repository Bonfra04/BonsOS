#include <shell.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

shell_t* shell_init()
{
    shell_t* shell = malloc(sizeof(shell_t));
    if (shell == NULL)
        return NULL;
    shell->length = 0;
    shell->offset = 0;
    shell->history = history_init();
    memset(shell->buff_left, 0, BUFFER_SIZE);
    memset(shell->buff_right, 0, BUFFER_SIZE);

    return shell;
}

void shell_free(shell_t* shell)
{
    history_free(shell->history);
    free(shell);
}

static void update_line(const shell_t* shell)
{
    if (shell->offset != shell->length)
    {
        for (size_t i = 0; i < shell->length; i++)
            putchar('\b');

        fputs(shell->buff_left, stdout);
        fputs(shell->buff_right, stdout);
    }
    else
        putchar(shell->buff_left[shell->offset]);
}

static void replace(shell_t* shell, const char* str)
{
    for (int i = 0; i < shell->length; i++)
        putchar('\b');

    memset(shell->buff_left, 0, BUFFER_SIZE);
    memset(shell->buff_right, 0, BUFFER_SIZE);

    strncpy(shell->buff_left, str, BUFFER_SIZE - 1);

    shell->length = strlen(shell->buff_left);
    shell->offset = shell->length;

    fputs(shell->buff_left, stdout);
}

void shell_delete(shell_t* shell)
{
    if (shell->length > 0)
    {
        shell->length--;
        shell->offset--;
        shell->buff_left[shell->offset] = '\b';
        update_line(shell);
        shell->buff_left[shell->offset] = '\0';
    }
}

void shell_submit(shell_t* shell)
{
    if (shell->length > 0)
    {
        if (shell->offset < shell->length)
        {
            strcat(shell->buff_left, shell->buff_right);
            memset(shell->buff_right, 0, BUFFER_SIZE);
        }

        history_add(shell->history, shell->buff_left);
        memset(shell->buff_left, 0, BUFFER_SIZE);

        shell->length = 0;
        shell->offset = 0;
    }

    putchar('\n');
}

void shell_move_cursor(shell_t* shell, bool direction /* true = right, false = left */)
{
    if(direction)
    {
        if (shell->offset < shell->length)
        {
            shell->buff_left[shell->offset] = shell->buff_right[0];
            memmove(shell->buff_right, shell->buff_right + 1, shell->offset - 1);
            shell->buff_right[shell->offset - 1] = '\0';
            shell->offset++;
        }
    }
    else
    {
        if (shell->offset > 0)
        {
            memmove(shell->buff_right + 1, shell->buff_right, shell->length - shell->offset);
            shell->offset--;
            shell->buff_right[0] = shell->buff_left[shell->offset];
            shell->buff_left[shell->offset] = '\0';
        }
    }

    // TODO: print char to move cursor [https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_(Control_Sequence_Introducer)_sequences]
}

void shell_handle_escape(shell_t* shell, escape_key_t key)
{
    switch (key)
    {
    case ARROW_UP:
        replace(shell, history_get_next(shell->history));
        break;
    case ARROW_DOWN:
        replace(shell, history_get_prev(shell->history));
        break;
    case ARROW_RIGHT:
        shell_move_cursor(shell, true);
        break;
    case ARROW_LEFT:
        shell_move_cursor(shell, false);
        break;
    }
}

void shell_print(shell_t* shell, char c)
{
    if (shell->length < BUFFER_SIZE && isprint(c))
    {
        shell->buff_left[shell->offset] = c;
        update_line(shell);
        shell->offset++;
        shell->length++;
    }
}
