#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include <syscalls.h>

#define BUFFER_SIZE 64

typedef enum escape_key
{
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    ARROW_LEFT,
    ESCAPE_NONE
} escape_key_t;

escape_key_t handle_escape()
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

typedef struct history
{
    char *commands[BUFFER_SIZE];
    int size;
    int index;
    bool has_scrolled_down;
} history_t;

history_t *init_history()
{
    history_t *history = malloc(sizeof(history_t));
    if (!history)
    {
        printf("Failed to allocate memory for history\n");
        exit(-1);
    }
    history->size = 0;
    history->index = 0;
    history->has_scrolled_down = false;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        history->commands[i] = calloc(BUFFER_SIZE, sizeof(char));
        if (!history->commands[i])
        {
            printf("Failed to allocate memory for history commands\n");
            exit(-1);
        }
    }
    return history;
}

void add_command(history_t *history, char *command)
{
    int i = 0;
    if (history->size == BUFFER_SIZE)
        i = BUFFER_SIZE - 1;
    else if (history->size > 0 && history->size < BUFFER_SIZE)
        i = history->size;

    for (; i > 0; i--)
        strncpy(history->commands[i], history->commands[i - 1], BUFFER_SIZE - 1);

    strncpy(history->commands[0], command, BUFFER_SIZE - 1);

    history->size += history->size != BUFFER_SIZE;
    history->index = 0;
    history->has_scrolled_down = false;
    return;
}

void print_history(history_t *history)
{
    for (int i = 0; i < history->size; i++)
    {
        printf("%d: %s\n", i, history->commands[i]);
    }
}

int main()
{
    freopen("tty:/raw", "r", stdin);

    char *buff_left = calloc(sizeof(char), BUFFER_SIZE);
    char *buff_right = calloc(sizeof(char), BUFFER_SIZE);
    if (!buff_left || !buff_right)
    {
        fputs("calloc of input buffer failed\n", stderr);
        return -1;
    }
    int length = 0;
    int offset = 0;

    history_t *history = init_history();

    while (1)
    {
        int input_char = getchar();

        if (input_char == EOF)
            continue;

        switch (input_char)
        {
        case 13:
            if (length > 0)
            {
                if (offset < length)
                {
                    strcat(buff_left, buff_right);
                    memset(buff_right, 0, BUFFER_SIZE);
                }

                add_command(history, buff_left);

                memset(buff_left, 0, BUFFER_SIZE);

                length = 0;
                offset = 0;
            }

            putchar('\n');
            break;
        case 127:
            if (offset > 0 && length > 0)
            {
                for (int i = 0; i < length; i++)
                    putchar('\b');

                length--;
                buff_left[--offset] = '\0';

                fputs(buff_left, stdout);
                fputs(buff_right, stdout);
            }
            break;
        case 27:
        {
            escape_key_t code = handle_escape();

            switch (code)
            {
            case ARROW_UP:
                if (history->index < history->size)
                {
                    if (length > 0)
                    {
                        for (int i = 0; i < length; i++)
                            putchar('\b');

                        memset(buff_left, 0, BUFFER_SIZE);
                    }

                    if (offset < length)
                        memset(buff_right, 0, BUFFER_SIZE);

                    history->index += history->has_scrolled_down && history->index == 0;
                    strncpy(buff_left, history->commands[history->index], BUFFER_SIZE - 1);
                    history->index += history->index != history->size - 1;

                    length = strlen(buff_left);
                    offset = length;

                    fputs(buff_left, stdout);
                }

                continue;
            case ARROW_DOWN:
                if (history->index > 0)
                {
                    if (length > 0)
                    {
                        for (int i = 0; i < length; i++)
                            putchar('\b');

                        memset(buff_left, 0, BUFFER_SIZE);
                    }

                    if (offset < length)
                        memset(buff_right, 0, BUFFER_SIZE);

                    history->index--;
                    strncpy(buff_left, history->commands[history->index], BUFFER_SIZE - 1);

                    length = strlen(buff_left);
                    offset = length;

                    history->has_scrolled_down = true;

                    fputs(buff_left, stdout);
                }

                continue;
            case ARROW_RIGHT:
                if (offset < length)
                {
                    buff_left[offset] = buff_right[0];
                    memmove(buff_right, buff_right + 1, offset - 1);
                    buff_right[offset - 1] = '\0';

                    offset++;
                }

                continue;
            case ARROW_LEFT:
                if (offset > 0)
                {
                    memmove(buff_right + 1, buff_right, length - offset);

                    buff_right[0] = buff_left[--offset];
                    buff_left[offset] = '\0';
                }

                continue;
            default:
                continue;
            }
        }
        default:
            if (length < BUFFER_SIZE && (input_char >= 32 && input_char <= 126))
            {
                buff_left[offset] = input_char;

                if (offset != length)
                {
                    for (int i = 0; i < length; i++)
                        putchar('\b');

                    fputs(buff_left, stdout);
                    fputs(buff_right, stdout);
                }
                else
                {
                    putchar(buff_left[offset]);
                }

                offset++;
                length++;
            }
        } // switch

    } // infinite loop

    return 0;
}