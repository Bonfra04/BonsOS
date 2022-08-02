#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct deque_node deque_node_t;
typedef struct deque_node {
    void* data;
    deque_node_t* next;
    deque_node_t* prev;
} deque_node_t;

typedef struct deque {
    deque_node_t* head;
    deque_node_t* tail;
    size_t size;
} deque_t;

deque_t deque_create();
#define deque() deque_create()

bool deque_push_front(deque_t* deque, void* data);
bool deque_push_back(deque_t* deque, void* data);
void* deque_pop_front(deque_t* deque);
void* deque_pop_back(deque_t* deque);

size_t deque_size(deque_t* deque);
void deque_clear(deque_t* deque);
