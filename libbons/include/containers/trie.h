#pragma once

#include <stdbool.h>

#define ALPHABET_SIZE (26+26+10+1) // A-Za-z0-9_

typedef struct trie_node trie_node_t;
typedef struct trie_node
{
    trie_node_t* children[ALPHABET_SIZE];
    trie_node_t* parent;
    void* value;
} trie_node_t;
typedef trie_node_t* trie_t;

trie_node_t* trie_gen_node(trie_node_t* parent);
#define trie() trie_gen_node(NULL)

void trie_destroy(trie_node_t* root);

bool trie_insert(trie_node_t* root, const char* key, void* value);
void trie_remove(trie_node_t* root, const char* key);

void* trie_get(const trie_node_t* root, const char* key);
