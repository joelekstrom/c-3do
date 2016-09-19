#include "textures.h"
#include "nano_bmp.h"

struct texture load_texture(char *file_name) {
	struct texture texture;
	bmp_t *bitmap = load_bmp(file_name);
	texture._internal = bitmap;
	texture.width = bitmap->info.w;
	texture.height = bitmap->info.h;
	return texture;
}

rgb_color texture_sample(struct texture t, vec2 coordinate) {
	rgb_color color;
	get_pixel(t._internal, (int)(coordinate.x * t.width), (int)(coordinate.y * t.height), &color.r, &color.g, &color.b);
	return color;
}
