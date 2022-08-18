#include <line.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

line_t* line_init()
{
    line_t* line = malloc(sizeof(line_t));
    if (line == NULL)
        return NULL;
    line->length = 0;
    line->offset = 0;
    line->history = history_init();
    memset(line->buff_left, 0, BUFFER_SIZE);
    memset(line->buff_right, 0, BUFFER_SIZE);

    return line;
}

void line_free(line_t* line)
{
    history_free(line->history);
    free(line);
}

static void update_line(const line_t* line)
{
    if (line->offset != line->length)
    {
        for (size_t i = 0; i < line->length; i++)
            putchar('\b');

        fputs(line->buff_left, stdout);
        fputs(line->buff_right, stdout);
    }
    else
        putchar(line->buff_left[line->offset]);
}

static void replace(line_t* line, const char* str)
{
    for (int i = 0; i < line->length; i++)
        putchar('\b');

    memset(line->buff_left, 0, BUFFER_SIZE);
    memset(line->buff_right, 0, BUFFER_SIZE);

    strncpy(line->buff_left, str, BUFFER_SIZE - 1);

    line->length = strlen(line->buff_left);
    line->offset = line->length;

    fputs(line->buff_left, stdout);
}

void line_delete(line_t* line)
{
    if (line->length > 0)
    {
        line->length--;
        line->offset--;
        line->buff_left[line->offset] = '\b';
        update_line(line);
        line->buff_left[line->offset] = '\0';
    }
}

char* line_submit(line_t* line)
{
    char* text = NULL;

    if (line->length > 0)
    {
        if (line->offset < line->length)
        {
            strcat(line->buff_left, line->buff_right);
            memset(line->buff_right, 0, BUFFER_SIZE);
        }

        history_add(line->history, line->buff_left);
        text = strdup(line->buff_left);
        memset(line->buff_left, 0, BUFFER_SIZE);

        line->length = 0;
        line->offset = 0;

        history_reset_cache(line->history);
    }
    putchar('\n');

    return text;
}

void line_move_cursor(line_t* line, bool direction /* true = right, false = left */)
{
    if(direction)
    {
        if (line->offset < line->length)
        {
            line->buff_left[line->offset] = line->buff_right[0];
            memmove(line->buff_right, line->buff_right + 1, line->offset - 1);
            line->buff_right[line->offset - 1] = '\0';
            line->offset++;
        }
    }
    else
    {
        if (line->offset > 0)
        {
            memmove(line->buff_right + 1, line->buff_right, line->length - line->offset);
            line->offset--;
            line->buff_right[0] = line->buff_left[line->offset];
            line->buff_left[line->offset] = '\0';
        }
    }

    // TODO: print char to move cursor [https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_(Control_Sequence_Introducer)_sequences]
}

void line_handle_escape(line_t* line, escape_key_t key)
{
    switch (key)
    {
    case ARROW_UP:
        replace(line, history_get_next(line->history));
        break;
    case ARROW_DOWN:
        replace(line, history_get_prev(line->history));
        break;
    case ARROW_RIGHT:
        line_move_cursor(line, true);
        break;
    case ARROW_LEFT:
        line_move_cursor(line, false);
        break;
    }
}

void line_print(line_t* line, char c)
{
    if (line->length < BUFFER_SIZE && isprint(c))
    {
        line->buff_left[line->offset] = c;
        update_line(line);
        line->offset++;
        line->length++;
    
        history_update_cache(line->history, c);
    }
}

static escape_key_t read_escape()
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

char* line_read(line_t* line)
{
    while(1)
    {
        int input_char = getchar();

        if (input_char == EOF)
            continue;

        switch (input_char)
        {
        case '\r':
            return line_submit(line);
        case 127:
            line_delete(line);
            break;
        case 27:
            line_handle_escape(line, read_escape());
            break;
        default:
            line_print(line, input_char);
        }
    }
}
