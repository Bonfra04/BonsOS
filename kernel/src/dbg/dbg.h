#pragma once

#include <stdbool.h>

#define dbg_break() asm volatile("int 3");

/**
 * @brief initialize the integrated debugger
 */
void dbg_init();