#include "graphics_context.h"
#include "nano_bmp.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

#define Z_BUFFER_NONE UINT_MAX

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

/**
 Fills a 2D triangle between 3 points.
 Uses the Barycentric algorithm. Slow and steady wins the race, right?
 */
void fill_triangle(vec2 p1, vec2 p2, vec2 p3, struct graphics_context *context, rgb_color color) {

	vec2 vertices[3] = { p1, p2, p3 };
	float min_x, min_y, max_x, max_y;
	get_bounding_box_2d(vertices, 3, &min_x, &min_y, &max_x, &max_y);

	// Spanning vectors between p1 and the other points
	vec2 vs1 = { p2.x - p1.x, p2.y - p1.y };
	vec2 vs2 = { p3.x - p1.x, p3.y - p1.y };

	for (int x = min_x; x <= max_x + 0.5; x++) {
		for (int y = min_y; y <= max_y + 0.5; y++) {
    		vec2 q = { x - p1.x, y - p1.y };

    		// Cross products to get intersections
    		 // return v1.x * v2.y - v1.y * v2.x;
    		float s = (q.x * vs2.y - q.y * vs2.x) / (vs1.x * vs2.y - vs1.y * vs2.x);
    		float t = (vs1.x * q.y - vs1.y * q.x) / (vs1.x * vs2.y - vs1.y * vs2.x);

    		// Check if point is inside triangle and draw
    		if ((s >= 0) && (t >= 0) && (s + t <= 1)) {
      			draw_pixel(x, y, context, color, NULL);
    		}
  		}
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
struct point_data {
	vec2 *point;
	rgb_color *color;
	float *depth;
};

/**
 Fills a goraud flat-bottomed triangle. Points/colors need to be sorted before, and
 bottom_left and bottom_right must have the same y-value, and it must be > top.y
 */
void flat_bottom_goraud(struct point_data top, 
						struct point_data bottom_left, 
						struct point_data bottom_right,
						struct graphics_context *context) 
{
	for (int y = top.point->y; y < bottom_left.point->y + 0.5; y++) {
		// Interpolate between top and bottom so we can calculate a line width
		float t = fmin((y - top.point->y) / (bottom_left.point->y - top.point->y), 1.0);

		// Calculate left and right points
		float left_x = bottom_left.point->x + ((top.point->x - bottom_left.point->x) * (1.0 - t));
		float width = (bottom_right.point->x - bottom_left.point->x) * t;
		float right_x = left_x + width;

		rgb_color line_left_color = interpolate_color(*top.color, *bottom_left.color, t);
		rgb_color line_right_color = interpolate_color(*top.color, *bottom_right.color, t);

		for (int x = roundf(left_x); x < roundf(right_x); x++) {
			float tx = (float)(x - left_x) / (float)width;

			// Calculate depth for z-buffering
			float pixel_depth = Z_BUFFER_NONE;
			float *depth_p = NULL;
			if (top.depth) {
				float left_depth = (*top.depth - *bottom_left.depth) * t + *bottom_left.depth;
				float right_depth = (*top.depth - *bottom_right.depth) * t + *bottom_right.depth;
				pixel_depth = (right_depth - left_depth) * tx + left_depth;
				depth_p = &pixel_depth;
			}

      		draw_pixel(x, y, context, interpolate_color(line_left_color, line_right_color, tx), depth_p);
		}
	}
}

/**
 Same as above but inverted
 */
void flat_top_goraud(struct point_data top_left, 
					 struct point_data top_right, 
					 struct point_data bottom,
					 struct graphics_context *context) 
{
	for (int y = bottom.point->y; y > top_left.point->y + 0.5; y--) {
		// Interpolate between top and bottom so we can calculate a line width
		float t = fmin(1.0 - (y - top_left.point->y) / (bottom.point->y - top_left.point->y), 1.0);

		// Calculate left and right points
		float left_x = top_left.point->x + ((bottom.point->x - top_left.point->x) * (1.0 - t));
		float width = (top_right.point->x - top_left.point->x) * t;
		float right_x = left_x + width;

		rgb_color line_left_color = interpolate_color(*bottom.color, *top_left.color, t);
		rgb_color line_right_color = interpolate_color(*bottom.color, *top_right.color, t);

		for (int x = roundf(left_x); x < roundf(right_x); x++) {
			float tx = (float)(x - left_x) / (float)width;

			// Calculate depth for z-buffering
			float pixel_depth = Z_BUFFER_NONE;
			float *depth_p = NULL;
			if (bottom.depth) {
				float left_depth = (*top_left.depth - *bottom.depth) * t + *bottom.depth;
				float right_depth = (*top_right.depth - *bottom.depth) * t + *bottom.depth;
				pixel_depth = (right_depth - left_depth) * tx + left_depth;
				depth_p = &pixel_depth;
			}

      		draw_pixel(x, y, context, interpolate_color(line_left_color, line_right_color, tx), depth_p);
		}
	}
}

int compare_points_x(const void *a, const void *b) {
	struct point_data *p1 = (struct point_data *)a;
	struct point_data *p2 = (struct point_data *)b;
	if (p1->point->x < p2->point->x) return -1;
	if (p1->point->x > p2->point->x) return 1;
	return 0;
}

int compare_points_y(const void *a, const void *b) {
	struct point_data *p1 = (struct point_data *)a;
	struct point_data *p2 = (struct point_data *)b;
	if (p1->point->y < p2->point->y) return -1;
	if (p1->point->y > p2->point->y) return 1;
	return 0;
}

int compare_points(const void *a, const void *b) {
	int cmp_y = compare_points_y(a, b);
	if (cmp_y != 0)
		return cmp_y;
	return compare_points_x(a, b);
}

/**
 Fills a goraud triangle (each vertex has a color which interpolates).
 This function sorts the points/colors and splits the triangle if needed,
 and then delegates drawing to flat_top/flat_bottom_goraud
 */
void goraud_triangle(vec2 vectors[3], rgb_color colors[3], struct graphics_context *context, float *point_depths) {

	// Build an array of point objects so we can sort vectors and colors together
	struct point_data points[3];
	for (int i = 0; i < 3; ++i) {
		struct point_data data;
		data.point = &vectors[i];
		data.color = &colors[i];
		data.depth = point_depths != NULL ? &point_depths[i] : NULL;
		points[i] = data;
	}

	// Sort it top->left->right
	qsort(&points, 3, sizeof(struct point_data), &compare_points);

	// If the y-value of the first and second points are the same, we have a flat-top triangle
	if (compare_points_y(&points[0], &points[1]) == 0) {
		flat_top_goraud(points[0], points[1], points[2], context);
	} 

	// ... And if the second and third have the same y-value, we have a flat-bottom triangle
	else if (compare_points_y(&points[1], &points[2]) == 0) {
		flat_bottom_goraud(points[0], points[1], points[2], context);
	} 

	// If the triangle has neither a flat top, or flat bottom, it makes it very complicated to draw.
	// Simplify it by splitting it into two triangles (one flat-top, and one flat-bottom)
	else {
		struct point_data split_point = points[1]; // We split the triangle on the middle-y point
		struct point_data other_points[] = { points[0], points[2] };

		// Interpolate between the 'other' points to create a new point
		float t = (split_point.point->y - other_points[0].point->y) / (other_points[1].point->y - other_points[0].point->y);
		vec2 new_point;
		new_point.y = split_point.point->y;
		new_point.x = flerp(other_points[0].point->x, other_points[1].point->x, t);
		rgb_color new_color = interpolate_color(*other_points[0].color, *other_points[1].color, t);
		float new_depth = flerp(*other_points[0].depth, *other_points[1].depth, t);

		// Call this function twice for two new, splitted triangles
		for (int i = 0; i < 2; i++) {
			vec2 points[] = {new_point, *split_point.point, *other_points[i].point};
			rgb_color colors[] = {new_color, *split_point.color, *other_points[i].color};
			float depths[] = {new_depth, *split_point.depth, *other_points[i].depth};
			goraud_triangle(points, colors, context, depths);
		}
	}
}

