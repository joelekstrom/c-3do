#include "graphics_context.h"
#include "../lib/nano-bmp/include/nano_bmp.h"
#include <stdio.h>
#include <math.h>

struct graphics_context *create_context(context_type type, int width, int height) {
	struct graphics_context *context = (struct graphics_context *)malloc(sizeof(struct graphics_context));
	context->type = type;
	context->width = width;
	context->height = height;

	if (type == BMP_CONTEXT_TYPE) {
		bmp_t *image = create_bmp(300, 300, 24);
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
	rgb_color result;
	result.r = c1.r + (c2.r - c1.r) * value;
	result.g = c1.g + (c2.g - c1.g) * value;
	result.b = c1.b + (c2.b - c1.b) * value;
	return result;
}

// ********** Drawing functions **********

void draw_pixel(int x, int y, struct graphics_context *context, rgb_color color) {
	if (context->type == BMP_CONTEXT_TYPE) {

		// Make sure pixel is within context bounds
		if (x > 0 && x <= context->width && y > 0 && y <= context->height) {
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
    		float s = cross_product_2d(q, vs2) / cross_product_2d(vs1, vs2);
    		float t = cross_product_2d(vs1, q) / cross_product_2d(vs1, vs2);

    		// Check if point is inside triangle and draw
    		if ((s >= 0) && (t >= 0) && (s + t <= 1)) {
      			draw_pixel(x, y, context, color);
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

    for (int x = p1.x; x <= p2.x; x++) { // Round to integers by adding 0.5
		float t = (x - p1.x) / (float)(p2.x - p1.x);
		int y = p1.y * (1.0 - t) + (p2.y * t) + 0.5;

		// De-transpose if needed
		int img_x = steep ? y : x;
		int img_y = steep ? x : y;

		draw_pixel(img_x, img_y, context, color);
	}
}

void clear(struct graphics_context *context, rgb_color color) {
	for (int x = 0; x < context->width; x++) {
		for (int y = 0; y < context->height; y++) {
			draw_pixel(x, y, context, color);
		}
	}
}

// ********** Goraud triangle drawing **********

/**
 Fills a goraud flat-bottomed triangle. Points/colors need to be sorted before, and
 bottom_left and bottom_right must have the same y-value, and it must be > top.y
 */
void flat_bottom_goraud(vec2 top, 
						vec2 bottom_left, 
						vec2 bottom_right, 
						rgb_color top_color, 
						rgb_color left_color, 
						rgb_color right_color, 
						struct graphics_context *context) 
{
	for (int y = top.y; y < bottom_left.y + 0.5; y++) {
		// Interpolate between top and bottom so we can calculate a line width
		float t = (y - top.y) / (bottom_left.y - top.y);

		// Calculate left and right points
		float left_x = bottom_left.x + ((top.x - bottom_left.x) * (1.0 - t));
		float width = (bottom_right.x - bottom_left.x) * t;
		float right_x = left_x + width;

		rgb_color line_left_color = interpolate_color(top_color, left_color, t);
		rgb_color line_right_color = interpolate_color(top_color, right_color, t);

		for (int x = (int)(left_x + 0.5); x < (int)(right_x + 0.5); x++) {
			float tx = (float)(x - left_x) / (float)width;
      		draw_pixel(x, y, context, interpolate_color(line_left_color, line_right_color, tx));
		}
	}
}

/**
 Same as above but inverted
 */
void flat_top_goraud(vec2 bottom, 
					 vec2 top_left, 
					 vec2 top_right, 
					 rgb_color bottom_color, 
					 rgb_color left_color, 
					 rgb_color right_color, 
					 struct graphics_context *context) 
{
	for (int y = bottom.y; y > top_left.y + 0.5; y--) {
		// Interpolate between top and bottom so we can calculate a line width
		float t = 1.0 - (y - top_left.y) / (bottom.y - top_left.y);

		// Calculate left and right points
		float left_x = top_left.x + ((bottom.x - top_left.x) * (1.0 - t));
		float width = (top_right.x - top_left.x) * t;
		float right_x = left_x + width;

		rgb_color line_left_color = interpolate_color(bottom_color, left_color, t);
		rgb_color line_right_color = interpolate_color(bottom_color, right_color, t);

		for (int x = (int)(left_x + 0.5); x < (int)(right_x + 0.5); x++) {
			float tx = (float)(x - left_x) / (float)width;
      		draw_pixel(x, y, context, interpolate_color(line_left_color, line_right_color, tx));
		}
	}
}

/**
 Fills a goraud triangle (each vertex has a color which interpolates).
 This function sorts the points/colors and splits the triangle if needed,
 and then delegates drawing to flat_bottom_goraud()
 */
void goraud_triangle(vec2 p1, vec2 p2, vec2 p3, rgb_color c1, rgb_color c2, rgb_color c3, struct graphics_context *context) {

	// We need to find the top, left and right points to make iteration simpler.
	// If the triangle isn't flat-bottomed we split it into two triangles
	vec2 *top_point = &p1;
	vec2 *left_point = NULL;
	vec2 *right_point = NULL;
	rgb_color *top_color = &c1;
	rgb_color *left_color = NULL;
	rgb_color *right_color = NULL;

	vec2 *points[] = { &p1, &p2, &p3 };
	rgb_color *colors[] = { &c1, &c2, &c3 };

	// First, find top point
	for (int i = 1; i < 3; ++i) {
		vec2 *p = points[i];
		if (p->y < top_point->y) {
			top_point = p;
			top_color = colors[i];
		}
	}

	// Then, find left and right point (we don't want them to be the same as top point)
	for (int i = 0; i < 3; i++) {
		vec2 *p = points[i];
		if (p == top_point)
			continue;

		if (left_point == NULL || p->x < left_point->x) {
			left_point = p;
			left_color = colors[i];
		}

		if (right_point == NULL || p->x > right_point->x) {
			right_point = p;
			right_color = colors[i];
		}
	}

	// If the y-value of the left and right points are the same, we already have a flat-bottom triangle
	if (right_point->y == left_point->y) {
		flat_bottom_goraud(*top_point, *left_point, *right_point, *top_color, *left_color, *right_color, context);
		return;
	}

	// If not, the triangle isn't flat bottomed, which makes it complicated to draw.
	// In that case, split it into two - one flat top and one flat bottom.
	if (right_point->y != left_point->y) {
		if (left_point->y < right_point->y) {
			vec2 split_point;
			split_point.y = left_point->y;
			float t = (left_point->y - top_point->y) / (right_point->y - top_point->y);
			split_point.x = (right_point->x - top_point->x) * t + top_point->x;
			rgb_color split_color = interpolate_color(*top_color, *right_color, t);
			flat_bottom_goraud(*top_point, *left_point, split_point, *top_color, *left_color, split_color, context);
			flat_top_goraud(*right_point, *left_point, split_point, *right_color, *left_color, split_color, context);
		} else {
			vec2 split_point;
			split_point.y = right_point->y;
			float t = (right_point->y - top_point->y) / (left_point->y - top_point->y);
			split_point.x = (top_point->x - left_point->x) * (1.0 - t) + left_point->x;
			rgb_color split_color = interpolate_color(*top_color, *left_color, t);
			flat_bottom_goraud(*top_point, split_point, *right_point, *top_color, split_color, *right_color, context);
			flat_top_goraud(*left_point, split_point, *right_point, *left_color, split_color, *right_color, context);
		}
	}
}

