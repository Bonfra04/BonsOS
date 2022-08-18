#pragma once

#include <history.h>

#include <stddef.h>
#include <stdbool.h>

typedef struct line
{
    char buff_left[BUFFER_SIZE];
    char buff_right[BUFFER_SIZE];
    size_t length;
    size_t offset;
    history_t* history;
} line_t;

typedef enum escape_key
{
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    ARROW_LEFT,
    ESCAPE_NONE
} escape_key_t;

line_t* line_init();
void line_free(line_t* line);

void line_delete(line_t* line);
char* line_submit(line_t* line);
void line_move_cursor(line_t* line, bool direction /* true = right, false = left */);
void line_handle_escape(line_t* line, escape_key_t key);
void line_print(line_t* line, char c);

char* line_read(line_t* line);
