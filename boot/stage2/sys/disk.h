#pragma once

#include "../lib/stdint.h"

void disk_init(uint8_t drive);

void disk_seek(uint32_t position);
void disk_read(uint32_t bytes, void* buffer);

