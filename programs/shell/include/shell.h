#pragma once

#include <history.h>

#include <stddef.h>
#include <stdbool.h>

typedef struct shell
{
    char buff_left[BUFFER_SIZE];
    char buff_right[BUFFER_SIZE];
    size_t length;
    size_t offset;
    history_t* history;
} shell_t;

typedef enum escape_key
{
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    ARROW_LEFT,
    ESCAPE_NONE
} escape_key_t;

shell_t* shell_init();
void shell_free(shell_t* shell);

void shell_delete(shell_t* shell);
void shell_submit(shell_t* shell);
void shell_move_cursor(shell_t* shell, bool direction /* true = right, false = left */);
void shell_handle_escape(shell_t* shell, escape_key_t key);
void shell_print(shell_t* shell, char c);
