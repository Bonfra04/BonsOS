#include "tga.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct rgba
{
	uint8_t r, g, b, a;
} __attribute__((packed)) rgba_t;

bool tga_load(tga_t* t, const char* path)
{
	FILE *fp = fopen(path, "r");

	if (!fp)
		return false;

	fread(&t->header.idlen, 1, 1, fp);
	fread(&t->header.colormaptype, 1, 1, fp);
	fread(&t->header.imagetype, 1, 1, fp);
	fread(&t->header.colormapoff, 2, 1, fp);
	fread(&t->header.colormaplen, 2, 1, fp);
	fread(&t->header.colormapdepth, 1, 1, fp);
	fread(&t->header.x, 2, 1, fp);
	fread(&t->header.y, 2, 1, fp);
	fread(&t->width, 2, 1, fp);
	fread(&t->height, 2, 1, fp);
	fread(&t->depth, 1, 1, fp);
	fread(&t->header.imagedesc, 1, 1, fp);

	t->size = t->width * t->height * sizeof(rgba_t);
	t->data = malloc(t->size);

	size_t bytes = (size_t)t->depth / 8;

	for (int i = 0; i < t->width * t->height; i++)
    {
		rgba_t p;
		fread(&p, bytes, 1, fp);
		t->data[i] = p.a << 24 | p.r << 16 | p.g << 8 | p.b;
	}

	fclose(fp);

	return true;
}
