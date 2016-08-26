#include "graphics_context.h"
#include "nano_bmp.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

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

// ********** Z-buffering ***************
float depth_buffer_get(int x, int y, struct graphics_context *context) {
	return context->depth_buffer[context->width * x + y];  
}

void depth_buffer_set(int x, int y, float value, struct graphics_context *context) {
	context->depth_buffer[context->width * x + y] = value;
}

// ********** Drawing functions **********

void draw_pixel(int x, int y, struct graphics_context *context, rgb_color color, float *depth) {
	if (context->type == BMP_CONTEXT_TYPE) {

		// Make sure pixel is within context bounds
		if (x > 0 && x <= context->width && y > 0 && y <= context->height) {
			
			// Perform depth check if needed
			if (depth != NULL) {
				float current_depth = depth_buffer_get(x, y, context);
				if (current_depth != Z_BUFFER_NONE && *depth > current_depth) 
					return;
				depth_buffer_set(x, y, *depth, context);
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

		draw_pixel(img_x, img_y, context, color, NULL);
	}
}

void clear(struct graphics_context *context, rgb_color color) {
	for (int x = 0; x < context->width; x++) {
		for (int y = 0; y < context->height; y++) {
			draw_pixel(x, y, context, color, NULL);
		}
	}
}

// ********** Goraud triangle drawing **********

/**
 A "tuple" which associates a vector with a color and depth. Comparing functions
 below so we can sort tuples easily top > left > right
 */
struct point {
	vec2 position;
	rgb_color color;
	vec2 texture_coordinate;
	float depth;
};

struct point interpolate_points(struct point a, struct point b, float value) {
	struct point result;
	result.position = vec2_lerp(a.position, b.position, value);
	result.color = interpolate_color(a.color, b.color, value);
	result.texture_coordinate = vec2_lerp(a.texture_coordinate, b.texture_coordinate, value);
	result.depth = flerp(a.depth, b.depth, value);
	return result;
}

void draw_point(struct point p, struct graphics_context *context, struct texture *texture) {
	rgb_color pixel_color;
	if (texture) {
		pixel_color = texture_sample(*texture, p.texture_coordinate);
	} else {
		pixel_color = p.color;
	}
	draw_pixel(roundf(p.position.x), roundf(p.position.y), context, pixel_color, &p.depth);
}

int compare_points_x(const void *a, const void *b) {
	struct point *p1 = (struct point *)a;
	struct point *p2 = (struct point *)b;
	if (p1->position.x < p2->position.x) return -1;
	if (p1->position.x > p2->position.x) return 1;
	return 0;
}

int compare_points_y(const void *a, const void *b) {
	struct point *p1 = (struct point *)a;
	struct point *p2 = (struct point *)b;
	if (p1->position.y < p2->position.y) return -1;
	if (p1->position.y > p2->position.y) return 1;
	return 0;
}

int compare_points(const void *a, const void *b) {
	int cmp_y = compare_points_y(a, b);
	if (cmp_y != 0)
		return cmp_y;
	return compare_points_x(a, b);
}

/**
 Fills a "flat triangle". Points/colors need to be sorted before, and
 left_leg and right_leg must have the same y-value.
 */
void flat_triangle(struct point anchor,
				   struct point left_leg,
				   struct point right_leg,
				   struct texture *texture,
				   struct graphics_context *context)
{
	int height = abs((int)roundf(anchor.position.y) - (int)roundf(left_leg.position.y));
	draw_point(anchor, context, texture);
		
	for (int y = 1; y <= height; y++) {
		float t = (float)y / (float)height;

		// Calculate left and right points
		struct point left_point = interpolate_points(anchor, left_leg, t);
		struct point right_point = interpolate_points(anchor, right_leg, t);
		int width = roundf(right_point.position.x) - roundf(left_point.position.x);

		for (int x = 0; x <= width; x++) {
			float tx = (float)x / (float)width;
			struct point point_to_draw = interpolate_points(left_point, right_point, tx);
			draw_point(point_to_draw, context, texture);
		}
	}
}

/**
 Fills a goraud triangle (each vertex has a color which interpolates).
 This function sorts the points/colors and splits the triangle if needed,
 and then delegates drawing to flat_triangle.
 */
void triangle_p(struct point points[3], struct texture *texture, struct graphics_context *context) {

	// Sort points it top->left->right
	qsort(points, 3, sizeof(struct point), &compare_points);

	// If the y-value of the first and second points are the same, we have a flat-top triangle
	if (compare_points_y(&points[0], &points[1]) == 0) {
		flat_triangle(points[2], points[0], points[1], texture, context);
	} 

	// ... And if the second and third have the same y-value, we have a flat-bottom triangle
	else if (compare_points_y(&points[1], &points[2]) == 0) {
		flat_triangle(points[0], points[1], points[2], texture, context);
	} 

	// If the triangle has neither a flat top, or flat bottom, it makes it very complicated to draw.
	// Simplify it by splitting it into two triangles (one flat-top, and one flat-bottom)
	else {
		struct point split_point = points[1]; // We split the triangle on the middle-y point
		struct point other_points[] = { points[0], points[2] };
		
		// Interpolate between the 'other' points to create a new point
		float t = (split_point.position.y - other_points[0].position.y) / (other_points[1].position.y - other_points[0].position.y);
		struct point new_point = interpolate_points(other_points[0], other_points[1], t);
		
		// Call this function twice for two new, splitted triangles
		for (int i = 0; i < 2; i++) {
			struct point points[] = {new_point, split_point, other_points[i]};
			triangle_p(points, texture, context);
		}
	}
}

/**
 The external interface for triangle drawings. Given arrays of vectors, colors, texture coords
 etc, it converts this data into struct point objects and delegates drawing to triangle_p.
 */
void triangle(vec2 vectors[3], rgb_color colors[3], struct texture *texture, vec2 texture_coordinates[3], struct graphics_context *context, float *point_depths) {
	struct point points[3];
	for (int i = 0; i < 3; ++i) {
		struct point data;
		data.position = vectors[i];
		data.color = colors[i];
		data.texture_coordinate = texture_coordinates[i];
		data.depth = point_depths[i];
		points[i] = data;
	}
	triangle_p(points, texture, context);
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
