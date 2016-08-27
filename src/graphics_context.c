#include "graphics_context.h"
#include "nano_bmp.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define Z_BUFFER_NONE UINT_MAX

rgb_color texture_sample(struct texture t, vec2 coordinate);

struct graphics_context *create_context(context_type type, int width, int height) {
	struct graphics_context *context = (struct graphics_context *)malloc(sizeof(struct graphics_context));
	context->depth_buffer = (float *)malloc(sizeof(float) * width * height);
	memset(context->depth_buffer, Z_BUFFER_NONE, sizeof(float) * width * height);
	context->type = type;
	context->width = width;
	context->height = height;

	if (type == BMP_CONTEXT_TYPE) {
		bmp_t *image = create_bmp(width, height, 24);
		context->_internal = image;
	} else {
		puts("Unsupported graphics context");
		exit(1);
	}
	return context;
}

void destroy_context(struct graphics_context *context) {
	if (context->type == BMP_CONTEXT_TYPE) {
		free(context->_internal);
		free(context->depth_buffer);
		free(context);
	}
}

void bmp_context_save(struct graphics_context *context, char name[]) {
	if (context->type == BMP_CONTEXT_TYPE) {
		write_bmp(name, context->_internal);
	} else {
		puts("Attempting to save non-bmp context");
		exit(EXIT_FAILURE);
	}
}

rgb_color multiply_color(rgb_color color, float value) {
	rgb_color result = { .r = color.r * value,
                         .g = color.g * value,
                         .b = color.b * value };
	return result;
}

rgb_color interpolate_color(rgb_color c1, rgb_color c2, float value) {
	rgb_color result = { .r = c1.r + (c2.r - c1.r) * value,
                         .g = c1.g + (c2.g - c1.g) * value,
                         .b = c1.b + (c2.b - c1.b) * value };
	return result;
}

/**
 Linearly interpolate, short "lerp", between two float values
 */
float flerp(float a, float b, float value) {
	return a + (b - a) * value;
}

vec2 vec2_lerp(vec2 a, vec2 b, float value) {
	vec2 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value)};
	return result;
}

vec3 vec3_lerp(vec3 a, vec3 b, float value) {
	vec3 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value), .z = flerp(a.z, b.z, value)};
	return result;
}

// ********** Z-buffering ***************
float depth_buffer_get(int x, int y, struct graphics_context *context) {
	return context->depth_buffer[context->width * x + y];  
}

void depth_buffer_set(int x, int y, float value, struct graphics_context *context) {
	context->depth_buffer[context->width * x + y] = value;
}

// ********** Drawing functions **********

void draw_pixel(vec3 coordinate, rgb_color color, bool z_buffering_enabled, struct graphics_context *context) {
	int x = (int)roundf(coordinate.x);
	int y = (int)roundf(coordinate.y);
	if (context->type == BMP_CONTEXT_TYPE) {

		// Make sure pixel is within context bounds
		if (x > 0 && x <= context->width && y > 0 && y <= context->height) {
			
			// Depth check (use the z-value for z-buffering)
			if (z_buffering_enabled) {
				float current_depth = depth_buffer_get(x, y, context);
				if (current_depth != Z_BUFFER_NONE && coordinate.z > current_depth) 
					return;
				depth_buffer_set(x, y, coordinate.z, context);
			}
			set_pixel(context->_internal, x, y, color.r, color.g, color.b);
		}
	} else {
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

	// We can only draw integrals, so round the numbers
	p1.x = (int)(p1.x + 0.5);
	p1.y = (int)(p1.y + 0.5);
	p2.x = (int)(p2.x + 0.5);
	p2.y = (int)(p2.y + 0.5);

	// If the line is steep (height > width), we transpose the line, so we can always loop on x-value
	int steep = fabsf(p2.y - p1.y) > fabsf(p2.x - p1.x);
    if (steep) {
		swapf(&p1.x, &p1.y);
		swapf(&p2.x, &p2.y);
    }

    // Make sure it's drawn left->right
    if (p2.x <= p1.x) {
		swapf(&p1.x, &p2.x);
		swapf(&p1.y, &p2.y);
    }

    for (int x = p1.x; x <= p2.x; x++) {
		float t = (x - p1.x) / (float)(p2.x - p1.x);
		int y = p1.y * (1.0 - t) + (p2.y * t) + 0.5;

		// De-transpose if needed
		int img_x = steep ? y : x;
		int img_y = steep ? x : y;

		struct vec3 coordinate = {.x = img_x, .y = img_y};
		draw_pixel(coordinate, color, false, context);
	}
}

void clear(struct graphics_context *context, rgb_color color) {
	for (int x = 0; x < context->width; x++) {
		for (int y = 0; y < context->height; y++) {
			vec3 coordinate = {.x = x, .y = y};
			draw_pixel(coordinate, color, false, context);
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
	draw_pixel(p.coordinate, color, true, context);
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
			  void (*vertex_shader)(struct vertex *v, void *input),
			  rgb_color (*fragment_shader)(struct vertex * const interpolated_v, void *input),
			  struct graphics_context *context)
{
	// Start by applying the vertex shader so we get correct coordinates/colors
	if (vertex_shader) {
		for (int i = 0; i < 3; i++) {
			vertex_shader(&vertices[i], shader_input);
		}
	}
	
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
			triangle(vertices, shader_input, vertex_shader, fragment_shader, context);
		}
	}
}

/******* Textures *********/

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
