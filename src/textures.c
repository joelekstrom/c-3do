#include "textures.h"
#include <SDL2/SDL.h>
#include <stdio.h>

struct texture load_texture(char *file_name) {
	struct texture texture;

	SDL_Surface *surface = SDL_LoadBMP(file_name);
	if (!surface) {
		fprintf(stderr, "Failed to load texture: %s", file_name);
		abort();
    }

	SDL_Surface *better_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
	SDL_FreeSurface(surface);

	texture.width = better_surface->w;
	texture.height = better_surface->h;

	size_t buffer_size = texture.width * texture.height * sizeof(int32_t);
	void *buffer = malloc(buffer_size);
	memcpy(buffer, better_surface->pixels, buffer_size);
	texture._internal = buffer;
	SDL_FreeSurface(better_surface);
	return texture;
}

void unload_texture(struct texture t) {
	free(t._internal);
}

rgb_color texture_sample(struct texture t, vec2 coordinate) {
	int x = (int)(coordinate.x * t.width);
	int y = (int)((1.0 - coordinate.y) * t.height);
	uint32_t *pixel_buffer = (uint32_t *)t._internal;
	uint32_t pixel = pixel_buffer[t.width * y + x];
	rgb_color color;
	color.r = pixel >> 24;
	color.g = pixel >> 16;
	color.b = pixel >> 8;
	return color;
}
