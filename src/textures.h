#ifndef TEXTURES_H
#define TEXTURES_H

#include "geometry.h"
#include "color.h"

struct texture {
	int width;
	int height;
	void *_internal;
};

struct texture load_texture(char *file_name);
rgb_color texture_sample(struct texture t, vec2 coordinate);

#endif
