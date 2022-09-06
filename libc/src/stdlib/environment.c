#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <containers/trie.h>

static trie_t variables;

void __environment_init(char* env)
{
    variables = trie();

    if(env)
    {
        for(char* decl = strtok(env, ";"); decl != NULL; decl = strtok(NULL, ";"))
        {
            char* sep = (char*)strchr(decl, '=');
            *sep = '\0';

            char* name = decl;
            char* value = sep + 1;

            trie_insert(variables, name, strdup(value));
        }
    }
}

char* getenv(const char* name)
{
    return trie_get(variables, name);
}

void setenv(const char* name, const char* val)
{
    free(trie_get(variables, name));
    trie_insert(variables, name, strdup(val));
}