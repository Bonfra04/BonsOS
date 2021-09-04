#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tga
{
	struct
    {
		uint8_t idlen;
		uint8_t colormaptype;
		uint8_t imagetype;
		uint16_t colormapoff;
		uint16_t colormaplen;
		uint8_t colormapdepth;
		uint16_t x, y;
		uint8_t imagedesc;
	} header;

	uint16_t width, height;
	uint8_t depth;
	size_t size;
	uint32_t* data;
} __attribute__((packed)) tga_t;

bool tga_load(tga_t* t, const char* path);
