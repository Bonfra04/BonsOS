#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* basename(const char* path);
char* dirname(char* path);
bool is_absolute(const char* path);
char* eval_path(const char* path);

#ifdef __cplusplus
}
#endif
