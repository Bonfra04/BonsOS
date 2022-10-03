#include "readline.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "escape.h"

typedef struct node
{
    char chr;
    struct node* next;
    struct node* prev;
} node_t;

static size_t last_render = 0;

static char* compact_list(size_t nchars, node_t* head)
{
    char* str = malloc(nchars + 1);
    node_t* cursor = head;
    for(size_t i = 0; i < nchars; i++)
    {
        str[i] = cursor->chr;
        node_t* next = cursor->next;
        free(cursor);
        cursor = next;
    }
    str[nchars] = '\0';
    return str;
}

static node_t* add_char(node_t* cursor, char ch)
{    
    node_t* new = malloc(sizeof(node_t));
    new->chr = ch;
    new->next = cursor;
    new->prev = cursor->prev;

    cursor->prev = new;
    if(new->prev)
        new->prev->next = new;

    return new;
}

static void remove_char(node_t* cursor)
{
    if(cursor->prev)
    {
        node_t* node = cursor->prev;

        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
    }
}

static void render_line(node_t* head, size_t nchars, node_t* cursor)
{
    node_t* ptr;

    ptr = head;
    for(size_t i = 0; i < nchars; i++)
    {
        fputc(ptr->chr, stdout);
        ptr = ptr->next;
    }
    
    while(ptr != cursor)
    {
        putchar('\b');
        ptr = ptr->prev;
    }

    last_render = nchars;
}

static void clear_line(size_t nchars, node_t* cursor)
{
    while(cursor->chr != '\0')
    {
        putchar(' ');
        cursor = cursor->next;
    }

    if(last_render > nchars)
    {
        for(size_t i = 0; i < last_render - nchars; i++)
            putchar('\b');
        for(size_t i = 0; i < last_render - nchars; i++)
            putchar(' ');
    }
    for(size_t i = 0; i < last_render; i++)
        fputs("\b \b", stdout);
}

char* readline()
{
    if(freopen("tty:/raw", "r", stdin) == NULL || freopen("tty:/raw", "w", stdout) == NULL)
        return NULL;

    // TODO: zero-out bss in executables (i guess idk)
    last_render = 0;
    
    node_t* cursor = calloc(sizeof(node_t), 1);
    node_t* head = add_char(cursor, (char)0xFF);
    size_t nchars = 0;

    char ch;
    while((ch = fgetc(stdin)) != '\r')
    {
        clear_line(nchars, cursor);

        switch (ch)
        {
        case 0x1b:
        {
            escape_t escape = readescape();
            switch (escape_simpify(&escape))
            {
                case ESCKB_ARROW_LEFT:
                    if(cursor->prev->chr != (char)0xFF)
                        cursor = cursor->prev;
                    break;
                case ESCKB_ARROW_RIGHT:
                    if(cursor->next)
                        cursor = cursor->next;
                    break;
            }
            break;
        }

        case 0x7F:
            if(nchars > 0)
            {
                remove_char(cursor);
                nchars--;
            }
            break;

        default:
            if(isprint(ch))
            {
                add_char(cursor, ch);
                nchars++;
            }
            // TODO control characters
            break;
        }

        render_line(head->next, nchars, cursor);
    }
    fputs("\n\r", stdout);

    char* str = compact_list(nchars, head->next);
    if(freopen("tty:/cooked", "r", stdin) == NULL || freopen("tty:/cooked", "w", stdout) == NULL)
    {
        free(str);
        return NULL;
    }
    return str;
}
