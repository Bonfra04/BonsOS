#include "path.h"
#include <string.h>
#include <stdint.h>
#include <ctype.h>

static bool is_character_valid(char c)
{
    return ('A' <= c && c <= 'Z')
           || ('a' <= c && c <= 'z')
           || ('0' <= c && c <= '9')
           || c == '!'
           || c == '#'
           || c == '$'
           || c == '%'
           || c == '&'
           || c == '('
           || c == ')'
           || c == '-'
           || c == '@'
           || c == '^'
           || c == '_'
           || c == '`'
           || c == '{'
           || c == '}'
           || c == '~';
}

bool to_short_filename(char* short_filename, const char* long_filename)
{
    if (!long_filename || !short_filename)
        return false;
    if(!long_filename[0])
        return false;

    if (long_filename[0] == '/')
        long_filename++;

    uint8_t separator;
    int i;
    // Find position of . (marker between name and extension)
    for (i = 0; i < 9; i++)
    {
        // If that is already the end of file, fill with spaces and exit
        if (!long_filename[i]) 
        {
            memset(&short_filename[i], ' ', 11 - i);
            return true;
        }

        if (long_filename[i] == '.')
        {
            separator = i;
            break;
        }

        if (!is_character_valid(long_filename[i]))
            return false;

        short_filename[i] = toupper(long_filename[i]);
    }
    
    // if there is no . more than 8 characters are forbidden
    if (i == 9)
        return false;

    memset(&short_filename[separator], ' ', 8 - separator);

    // Copy extension
    for (i = 0; i < 3; i++) {
        if (!long_filename[separator + 1 + i])
            break;

        if (!is_character_valid(long_filename[separator + 1 + i]))
            return false;

        short_filename[8 + i] = toupper(long_filename[separator + 1 + i]);
    }

    // Check that extension consists of no more than 3 characters
    if (i == 3 && long_filename[separator + 4])
        return false;

    memset(&short_filename[8 + i], ' ', 3 - i);

    return true;
}

bool get_subdir(char* subdir_name, size_t* index, const char* path)
{
    size_t beg = *index;
    size_t len = 0;

    if (path[beg] != '/')
        return false;

    len++; // Skip first slash
    while (path[beg + len] != '/' && path[beg + len])
        ++len;

    // Check if path is an intermediate directory
    if (path[beg + len] != '/')
        return false;

    if (len > 12)
        return false;

    memcpy(subdir_name, &path[beg], len);
    subdir_name[len] = '\0';

    *index = beg + len;
    return true;
}

bool is_in_root(const char* path)
{
    char subdir_name[13];
    size_t index = 0;

    return !get_subdir(subdir_name, &index, path);
}