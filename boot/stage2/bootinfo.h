#pragma once

#include "lib/stdint.h"

typedef struct
{
    uint32_t memoryMapAddress;
    uint32_t memoryMapEntries;
    uint64_t memory_size;
    uint32_t bootDevice;
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t screen_pitch;
    uint32_t framebuffer;
} bootinfo_t;