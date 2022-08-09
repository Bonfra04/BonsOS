#include <containers/trie.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

trie_node_t* trie_gen_node(trie_node_t* parent)
{
    trie_node_t* node = malloc(sizeof(trie_node_t));
    if(node)
    {
        memset(node, 0, sizeof(trie_node_t));
        node->parent = parent;
    }
    return node;
}

void trie_destroy(trie_node_t* root)
{
    for(size_t i = 0; i < ALPHABET_SIZE; i++)
        if(root->children[i])
            trie_destroy(root->children[i]);
    free(root);
}

bool trie_insert(trie_node_t* root, const char* key, void* value)
{
    trie_node_t* node = root;
    size_t len = strlen(key);

    for(uint64_t i = 0; i < len; i++)
    {
        uint64_t index = key[i] - 'a';
        if(node->children[index] == NULL)
            if((node->children[index] = trie_gen_node(node)) == NULL)
                return false;
        node = node->children[index];
    }

    node->value = value;
    return true;
}

static const trie_node_t* find_node(const trie_node_t* root, const char* key)
{
    const trie_node_t* node = root;
    size_t len = strlen(key);

    for(uint64_t i = 0; i < len; i++)
        if((node = node->children[key[i] - 'a']) == NULL)
            return NULL;

    return node;
}

static bool has_children(trie_node_t* node)
{
    for(uint64_t i = 0; i < ALPHABET_SIZE; i++)
        if(node->children[i] != NULL)
            return true;
    return false;
}

static void delete_node(trie_node_t* node)
{
    trie_node_t* parent = node->parent;
    for(uint64_t i = 0; i < ALPHABET_SIZE; i++)
        if(parent->children[i] == node)
        {
            parent->children[i] = NULL;
            break;
        }


    free(node);
    if(parent->value == NULL && !has_children(parent))
        delete_node(parent);
}

void trie_remove(trie_node_t* root, const char* key)
{
    trie_node_t* node = (trie_node_t*)find_node(root, key);
    if(node == NULL)
        return;

    node->value = NULL;

    if(!has_children(node))
        delete_node(node);
}

void* trie_get(const trie_node_t* root, const char* key)
{
    const trie_node_t* node = find_node(root, key);
    if(node == NULL)
        return NULL;
    return node->value;
}
