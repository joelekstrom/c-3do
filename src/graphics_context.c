#include "graphics_context.h"
#include "nano_bmp.h"
#include "textures.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define Z_BUFFER_NONE UINT_MAX

rgb_color texture_sample(struct texture t, vec2 coordinate);

struct graphics_context *create_context(context_type type, int width, int height, void (*render_callback)(struct graphics_context *context)) {
	struct graphics_context *context = (struct graphics_context *)malloc(sizeof(struct graphics_context));
	context->depth_buffer = (float *)malloc(sizeof(float) * width * height);
	memset(context->depth_buffer, Z_BUFFER_NONE, sizeof(float) * width * height);
	context->type = type;
	context->width = width;
	context->height = height;
	context->render_callback = render_callback;

	if (type == BMP_CONTEXT_TYPE) {
		bmp_t *image = create_bmp(width, height, 24);
		context->_internal = image;
	} else if (type == WINDOW_CONTEXT_TYPE) {
		if (SDL_Init(SDL_INIT_VIDEO) != 0){
			printf("SDL_Init Error: %s\n", SDL_GetError());
			SDL_Quit();
		}

		SDL_Window *window = SDL_CreateWindow("c3do", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
		if (window == NULL) {
			printf("Could not create window: %s\n", SDL_GetError());
		}
		context->_internal = window;
	}
	return context;
}

void destroy_context(struct graphics_context *context) {
	if (context->type == BMP_CONTEXT_TYPE) {
		free(context->_internal);
		free(context->depth_buffer);
		free(context);
	} else if (context->type == WINDOW_CONTEXT_TYPE) {
		SDL_DestroyWindow(context->_internal);
	}
}

void context_activate(struct graphics_context *context) {
	if (context->type == BMP_CONTEXT_TYPE) {
		context->render_callback(context);
		write_bmp("output.bmp", context->_internal);
	} else if (context->type == WINDOW_CONTEXT_TYPE) {
		SDL_Delay(3000);
	}
}

// ********** Z-buffering ***************
float depth_buffer_get(int x, int y, struct graphics_context *context) {
	return context->depth_buffer[context->width * x + y];
}

void depth_buffer_set(int x, int y, float value, struct graphics_context *context) {
	context->depth_buffer[context->width * x + y] = value;
}

// ********** Drawing functions **********

void draw_fragment(vec3 coordinate, rgb_color color, struct graphics_context *context) {
	int x = (int)roundf(coordinate.x);
	int y = (int)roundf(coordinate.y);
	if (context->type == BMP_CONTEXT_TYPE) {

		// Discard fragments outside buffer bounds
		if (x < 0 || x >= context->width || y < 0 || y >= context->height) {
			return;
		}
			
		// Depth check (use the z-value for z-buffering)
		if (context->depth_buffer) {
			float current_depth = depth_buffer_get(x, y, context);
			if (current_depth != Z_BUFFER_NONE && coordinate.z > current_depth) 
				return;
			depth_buffer_set(x, y, coordinate.z, context);
		}
		set_pixel(context->_internal, x, y, color.r, color.g, color.b);
	}  else {
		puts("Unsupported graphics context");
		exit(EXIT_FAILURE);	
	}
}

void swapf(float *a, float *b) {
	float tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 Draws a 2D line between two points, ignoring Z-value
 */
void draw_line(vec2 p1, vec2 p2, struct graphics_context *context, rgb_color color) {
	float line_width = fabsf(p2.x - p1.x);
	float line_height = fabsf(p2.y - p1.y);
	float length = (line_width > line_height) ? line_width : line_height;
	
    for (int i = 0; i < roundf(length); i++) {
		float t = (float)i / length;
		vec2 p = vec2_lerp(p1, p2, t);

		struct vec3 coordinate = {.x = p.x, .y = p.y, .z = -9000.0};
		draw_fragment(coordinate, color, context);
	}
}

void clear(struct graphics_context *context, rgb_color color) {
	for (int x = 0; x < context->width; x++) {
		for (int y = 0; y < context->height; y++) {
			vec3 coordinate = {.x = x, .y = y};
			draw_fragment(coordinate, color, context);
		}
	}
}

// ********** Goraud triangle drawing **********

struct vertex vertex_lerp(struct vertex a, struct vertex b, float value) {
	struct vertex result;
	result.coordinate = vec3_lerp(a.coordinate, b.coordinate, value);
	result.color = interpolate_color(a.color, b.color, value);
	result.normal = vec3_lerp(a.normal, b.normal, value);
	result.texture_coordinate = vec2_lerp(a.texture_coordinate, b.texture_coordinate, value);
	return result;
}

void draw_point(struct vertex p, void *shader_input, rgb_color (*fragment_shader)(struct vertex * const interpolated_v, void *input), struct graphics_context *context) {
	rgb_color color = fragment_shader ? fragment_shader(&p, shader_input) : p.color;
	draw_fragment(p.coordinate, color, context);
}

int compare_vertices_x(const void *a, const void *b) {
	struct vertex *p1 = (struct vertex *)a;
	struct vertex *p2 = (struct vertex *)b;
	if (p1->coordinate.x < p2->coordinate.x) return -1;
	if (p1->coordinate.x > p2->coordinate.x) return 1;
	return 0;
}

int compare_vertices_y(const void *a, const void *b) {
	struct vertex *p1 = (struct vertex *)a;
	struct vertex *p2 = (struct vertex *)b;
	if (p1->coordinate.y < p2->coordinate.y) return -1;
	if (p1->coordinate.y > p2->coordinate.y) return 1;
	return 0;
}

int compare_vertices(const void *a, const void *b) {
	int cmp_y = compare_vertices_y(a, b);
	if (cmp_y != 0)
		return cmp_y;
	return compare_vertices_x(a, b);
}

/**
 Fills a "flat triangle". Points/colors need to be sorted before, and
 left_leg and right_leg must have the same y-value.
 */
void flat_triangle(struct vertex anchor,
				   struct vertex left_leg,
				   struct vertex right_leg,
				   void *shader_input,
				   rgb_color (*fragment_shader)(struct vertex * const interpolated_v, void *input),
				   struct graphics_context *context)
{
	int height = abs((int)roundf(anchor.coordinate.y) - (int)roundf(left_leg.coordinate.y));
	draw_point(anchor, shader_input, fragment_shader, context);
		
	for (int y = 1; y <= height; y++) {
		float t = (float)y / (float)height;

		// Calculate left and right points
		struct vertex left_point = vertex_lerp(anchor, left_leg, t);
		struct vertex right_point = vertex_lerp(anchor, right_leg, t);
		int width = roundf(right_point.coordinate.x) - roundf(left_point.coordinate.x);

		for (int x = 0; x <= width; x++) {
			float tx = (float)x / (float)width;
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
			  void *shader_input,
			  rgb_color (*fragment_shader)(struct vertex * const interpolated_v, void *input),
			  struct graphics_context *context)
{	
	// Sort points it top->left->right
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
		float t = (split_point.coordinate.y - other_vertices[0].coordinate.y) / (other_vertices[1].coordinate.y - other_vertices[0].coordinate.y);
		struct vertex new_point = vertex_lerp(other_vertices[0], other_vertices[1], t);
		
		// Call this function twice for two new, splitted triangles
		for (int i = 0; i < 2; i++) {
			struct vertex vertices[] = {new_point, split_point, other_vertices[i]};
			triangle(vertices, shader_input, fragment_shader, context);
		}
	}
}
