#pragma once

#include <stdbool.h>
#include <stddef.h>

#define BUFFER_SIZE 64

typedef struct history
{
    char* commands[BUFFER_SIZE];
    size_t size;
    size_t index;
} history_t;

history_t* history_init();
void history_free(history_t* history);
void history_add(history_t* history, char* command);
const char* history_get_next(history_t* history);
const char* history_get_prev(history_t* history);
