#include "graphics_context.h"
#include "nano-bmp/include/nano_bmp.h"
#include <stdio.h>

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
		exit(1);
	}
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
		exit(1);	
	}
}

/**
 Fills a 2D triangle between 3 points, ignoring Z value.
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
	int steep = abs(p2.y - p1.y) > abs(p2.x - p1.x);
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

		if (x < 0) {
			printf("x is below 0: %i", x);
		}

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