#include "textures.h"
#include <SDL2/SDL.h>

struct texture load_texture(char *file_name) {
	struct texture texture;

	SDL_Surface *surface = SDL_LoadBMP(file_name);
	SDL_Surface *better_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
	SDL_FreeSurface(surface);

	texture.width = better_surface->w;
	texture.height = better_surface->h;
	texture._internal = better_surface;

	return texture;
}

rgb_color texture_sample(struct texture t, vec2 coordinate) {
	rgb_color color;
	int x = (int)(coordinate.x * t.width);
	int y = (int)((1.0 - coordinate.y) * t.height);

	SDL_Surface *surface = t._internal;
	uint32_t pixel = *((uint32_t *)surface->pixels + y * surface->w + x);
	SDL_GetRGB(pixel, surface->format, &color.r, &color.g, &color.b);
	return color;
}
