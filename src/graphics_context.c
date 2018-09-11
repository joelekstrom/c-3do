
#include "graphics_context.h"
#include "textures.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define Z_BUFFER_NONE UINT_MAX

rgb_color texture_sample(struct texture t, vec2 coordinate);

struct graphics_context *create_context(int width, int height) {
	struct graphics_context *context = (struct graphics_context *)malloc(sizeof(struct graphics_context));
	context->depth_buffer = (float *)malloc(sizeof(float) * width * height);
	context->pixel_buffer = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
	memset(context->depth_buffer, Z_BUFFER_NONE, sizeof(float) * width * height);
	context->width = width;
	context->height = height;
	context->window_event_callback = NULL;
	context->_internal = NULL;
	return context;
}

void destroy_context(struct graphics_context *context) {
	if (context->_internal) {
		SDL_DestroyWindow(context->_internal);
	}
	free(context->pixel_buffer);
	free(context->depth_buffer);
	free(context);
}

SDL_Surface *create_surface_from_context(struct graphics_context *context) {
	return SDL_CreateRGBSurfaceFrom(context->pixel_buffer,
									context->width,
									context->height,
									32,
									context->width * sizeof(uint32_t),
									0xff000000, 0x00ff0000, 0x0000ff00, 0);
}

void context_activate_window(struct graphics_context *context) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("SDL_Init Error: %s\n", SDL_GetError());
		SDL_Quit();
	}

	SDL_Window *window = SDL_CreateWindow("c3do", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, context->width, context->height, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		printf("Could not create window: %s\n", SDL_GetError());
	}
	context->_internal = window;

	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_QUIT)
			return;

		if (context->window_event_callback) {
			context->window_event_callback(context, event);
		}
	}
}

void context_refresh_window(struct graphics_context *context) {
	SDL_Surface *surface = create_surface_from_context(context);
	SDL_BlitSurface(surface, NULL, SDL_GetWindowSurface(context->_internal), NULL);
	SDL_UpdateWindowSurface(context->_internal);
	SDL_FreeSurface(surface);
}

void context_save_BMP(struct graphics_context *context, char file_name[]) {
	SDL_Surface *surface = create_surface_from_context(context);
	SDL_SaveBMP(surface, file_name);
	SDL_FreeSurface(surface);
}

// ********** Z-buffering ***************
double depth_buffer_get(int x, int y, struct graphics_context *context) {
	return context->depth_buffer[context->width * x + y];
}

void depth_buffer_set(int x, int y, double value, struct graphics_context *context) {
	context->depth_buffer[context->width * x + y] = value;
}

// ********** Drawing functions **********

void draw_fragment(vec3 coordinate, rgb_color color, struct graphics_context *context) {
	int x = (int)round(coordinate.x);
	int y = (int)round(coordinate.y);

	// Discard fragments outside buffer bounds
	if (x < 0 || x >= context->width || y < 0 || y >= context->height) {
		return;
	}
			
	// Depth check (use the z-value for z-buffering)
	if (context->depth_buffer) {
		double current_depth = depth_buffer_get(x, y, context);
		if (current_depth != Z_BUFFER_NONE && coordinate.z > current_depth)
			return;
		depth_buffer_set(x, y, coordinate.z, context);
	}

	context->pixel_buffer[context->width * y + x] = rgba_from_color(color);
}

void swapf(double *a, double *b) {
	double tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 Draws a 2D line between two points, ignoring Z-value
 */
void draw_line(vec2 p1, vec2 p2, struct graphics_context *context, rgb_color color) {
	double line_width = fabs(p2.x - p1.x);
	double line_height = fabs(p2.y - p1.y);
	double length = (line_width > line_height) ? line_width : line_height;
	
    for (int i = 0; i < round(length); i++) {
		double t = (double)i / length;
		vec2 p = lerp(p1, p2, t);

		struct vec3 coordinate = {.x = p.x, .y = p.y, .z = -9000.0};
		draw_fragment(coordinate, color, context);
	}
}

void clear(struct graphics_context *context, rgb_color color) {
	// Clear Z-buffer
	memset(context->depth_buffer, Z_BUFFER_NONE, sizeof(double) * context->width * context->height);
	uint32_t rgba = rgba_from_color(color);
	for (int i = 0; i < context->width * context->height; i++) {
		context->pixel_buffer[i] = rgba;
	}
}

struct vertex vertex_lerp(struct vertex a, struct vertex b, double value) {
	struct vertex result;
	result.coordinate = lerp(a.coordinate, b.coordinate, value);
	result.color = interpolate_color(a.color, b.color, value);
	result.normal = lerp(a.normal, b.normal, value);
	result.texture_coordinate = lerp(a.texture_coordinate, b.texture_coordinate, value);
	return result;
}

void draw_point(struct vertex p, struct fragment_shader_input shader_input, rgb_color (*fragment_shader)(struct fragment_shader_input), struct graphics_context *context) {
	shader_input.interpolated_v = p;
	rgb_color color = fragment_shader ? fragment_shader(shader_input) : p.color;
	draw_fragment(p.coordinate, color, context);
}

int compare_vertices_x(const void *a, const void *b) {
	struct vertex *p1 = (struct vertex *)a;
	struct vertex *p2 = (struct vertex *)b;
	double x1 = round(p1->coordinate.x);
	double x2 = round(p2->coordinate.x);
	if (x1 < x2) return -1;
	if (x1 > x2) return 1;
	return 0;
}

int compare_vertices_y(const void *a, const void *b) {
	struct vertex *p1 = (struct vertex *)a;
	struct vertex *p2 = (struct vertex *)b;
	double y1 = round(p1->coordinate.y);
	double y2 = round(p2->coordinate.y);
	if (y1 < y2) return -1;
	if (y1 > y2) return 1;
	return 0;
}

int compare_vertices(const void *a, const void *b) {
	int cmp_y = compare_vertices_y(a, b);
	if (cmp_y != 0)
		return cmp_y;
	return compare_vertices_x(a, b);
}

/**
 Calculates a rectangular bounding box for a triangle, and
 returns true if the triangle is visible within context bounds.
 */
bool triangle_intersects_bounds(struct vertex vertices[3], struct graphics_context *context) {
	double top = fminf(fminf(vertices[0].coordinate.y, vertices[1].coordinate.y), vertices[2].coordinate.y);
	double left = fminf(fminf(vertices[0].coordinate.x, vertices[1].coordinate.x), vertices[2].coordinate.x);
	double bottom = fmaxf(fmaxf(vertices[0].coordinate.y, vertices[1].coordinate.y), vertices[2].coordinate.y);
	double right = fmaxf(fmaxf(vertices[0].coordinate.x, vertices[1].coordinate.x), vertices[2].coordinate.x);
	return !(right < 0 || bottom < 0 || left > context->width || top > context->height);
}

/**
 Fills a "flat triangle". Points/colors need to be sorted before, and
 left_leg and right_leg must have the same y-value.
 */
void flat_triangle(struct vertex anchor,
				   struct vertex left_leg,
				   struct vertex right_leg,
				   struct fragment_shader_input shader_input,
				   rgb_color (*fragment_shader)(struct fragment_shader_input),
				   struct graphics_context *context)
{
	int height = abs((int)round(anchor.coordinate.y) - (int)round(left_leg.coordinate.y));
	draw_point(anchor, shader_input, fragment_shader, context);

	for (int y = 1; y <= height; y++) {
		double t = (double)y / (double)height;

		// Calculate left and right points
		struct vertex left_point = vertex_lerp(anchor, left_leg, t);
		struct vertex right_point = vertex_lerp(anchor, right_leg, t);
		int width = round(right_point.coordinate.x) - round(left_point.coordinate.x);
		if (width == 0) {
			continue;
		}

		for (int x = 0; x <= width; x++) {
			double tx = (double)x / (double)width;
			struct vertex point_to_draw = vertex_lerp(left_point, right_point, tx);
			draw_point(point_to_draw, shader_input, fragment_shader, context);
		}
	}
}

/**
 Fills a goraud triangle (each vertex has a color which interpolates).
 This function sorts the points/colors and splits the triangle if needed,
 and then delegates drawing to flat_triangle.
 */
void triangle(struct vertex vertices[3],
			  struct fragment_shader_input shader_input,
			  rgb_color (*fragment_shader)(struct fragment_shader_input),
			  struct graphics_context *context)
{
	// Ignore triangle if it won't be visible
	if (!triangle_intersects_bounds(vertices, context)) {
		return;
	}

	// Sort points top->left->right
	qsort(vertices, 3, sizeof(struct vertex), &compare_vertices);

	// If the y-value of the first and second vertices are the same, we have a flat-top triangle
	if (compare_vertices_y(&vertices[0], &vertices[1]) == 0) {
		flat_triangle(vertices[2], vertices[0], vertices[1], shader_input, fragment_shader, context);
	} 

	// ... And if the second and third have the same y-value, we have a flat-bottom triangle
	else if (compare_vertices_y(&vertices[1], &vertices[2]) == 0) {
		flat_triangle(vertices[0], vertices[1], vertices[2], shader_input, fragment_shader, context);
	} 

	// If the triangle has neither a flat top, or flat bottom, it makes it very complicated to draw.
	// Simplify it by splitting it into two triangles (one flat-top, and one flat-bottom)
	else {
		struct vertex split_point = vertices[1]; // We split the triangle on the middle-y point
		struct vertex other_vertices[] = {vertices[0], vertices[2]};
		
		// Interpolate between the 'other' vertices to create a new point
		double t = (split_point.coordinate.y - other_vertices[0].coordinate.y) / (other_vertices[1].coordinate.y - other_vertices[0].coordinate.y);
		struct vertex new_point = vertex_lerp(other_vertices[0], other_vertices[1], t);
		
		// Call this function for each of the splitted triangles
		triangle((struct vertex[]){new_point, split_point, other_vertices[0]}, shader_input, fragment_shader, context);
        triangle((struct vertex[]){new_point, split_point, other_vertices[1]}, shader_input, fragment_shader, context);
    }
}
