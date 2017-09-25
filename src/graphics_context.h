#ifndef GRAPHICS_CONTEXT_H
#define GRAPHICS_CONTEXT_H

#include "geometry.h"
#include "color.h"
#include "shaders.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

struct graphics_context {
	int width;
	int height;
	uint32_t *pixel_buffer;
	float *depth_buffer;
	void (*window_event_callback)(struct graphics_context *context, SDL_Event event);
	void *_internal;
};

struct graphics_context *create_context(int width, int height);
void context_activate_window(struct graphics_context *context);
void context_refresh_window(struct graphics_context *context);
void context_save_BMP(struct graphics_context *context, char file_name[]);
void destroy_context(struct graphics_context *context);

void draw_line(vec2 p1, vec2 p2, struct graphics_context *context, rgb_color color);
void clear(struct graphics_context *context, rgb_color color);

/**
 Draws a 2D triangle. Uses the Z-value of the coordinates for Z-buffering,
 so the Z-value has no visual meaning, and is only used if the graphics
 context has a Z-buffer enabled.
 */
void triangle(struct vertex vertices[3],
			  struct fragment_shader_input shader_input,
			  rgb_color (*fragment_shader)(struct fragment_shader_input input),
			  struct graphics_context *context);

#endif
