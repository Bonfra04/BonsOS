#pragma once

#include "../lib/stdint.h"
#include "../lib/stdbool.h"

typedef struct vesa_descriptor
{
    uint16_t width;
    uint16_t height;
    uint8_t depth;
    uint16_t pitch;
    uint16_t id;
    void* framebuffer;
} __attribute__((packed)) vesa_descriptor_t;

void vesa_enable_mode(uint16_t mode);
bool vesa_find_mode(uint16_t width, uint16_t height, uint8_t depth, vesa_descriptor_t* vesa_desc);
