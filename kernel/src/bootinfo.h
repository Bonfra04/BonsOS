#pragma once

#include <stdint.h>

typedef struct
{
    uint32_t memoryMapAddress;
    uint32_t memoryMapEntries;
    uint32_t memorySizeLow;     // number of KB above 1MB
    uint32_t memorySizeHigh;    // number of 64KB blocks above 16MB
    uint32_t bootDevice;
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t screen_pitch;
    uint32_t framebuffer;
} bootinfo_t;