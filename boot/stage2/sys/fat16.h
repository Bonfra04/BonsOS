#pragma once

#include "stdbool.h"
#include "../lib/stdint.h"

void fsys_init(uint32_t partition_offset);
bool file_read(const char* name, void* address);