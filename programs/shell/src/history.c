#include <history.h>

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

history_t* history_init()
{
    history_t* history = malloc(sizeof(history_t));
    if (history == NULL)
        return NULL;

    history->size = 0;
    history->index = -1;
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        history->commands[i] = calloc(BUFFER_SIZE, sizeof(char));
        if (history->commands[i] == NULL)
        {
            history_free(history);
            return NULL;
        }
    }

    memset(history->cache, 0, BUFFER_SIZE);
    history->cache_index = 0;

    return history;
}

void history_free(history_t* history)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
        free(history->commands[i]);
    free(history);
}

void history_add(history_t* history, char* command)
{
    size_t i = 0;
    if (history->size == BUFFER_SIZE)
        i = BUFFER_SIZE - 1;
    else if (history->size > 0 && history->size < BUFFER_SIZE)
        i = history->size;

    for (; i > 0; i--)
        strncpy(history->commands[i], history->commands[i - 1], BUFFER_SIZE - 1);

    strncpy(history->commands[0], command, BUFFER_SIZE - 1);

    history->size += history->size != BUFFER_SIZE;
    history->index = -1;
}

const char* history_get_next(history_t* history)
{
    if(history->size == 0)
        return NULL;
    history->index += history->index != history->size - 1;
    char* command = history->commands[history->index];
    return command;
}

const char* history_get_prev(history_t* history)
{
    if(history->size == 0)
        return NULL;
    if(history->index == 0)
        return history->cache;

    history->index -= history->index != 0;
    char* command = history->commands[history->index];
    return command;
}

void history_update_cache(history_t* history, char c) 
{
    history->cache[history->cache_index++] = c;
}

void history_reset_cache(history_t* history) {
    memset(history->cache, 0, BUFFER_SIZE);
    history->cache_index = 0;
}