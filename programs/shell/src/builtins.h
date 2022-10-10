#pragma once

#define NUM_BUILTINS 2

extern const char* builtin_str[NUM_BUILTINS];
extern int (*builtin_func[NUM_BUILTINS])(const char** args);
