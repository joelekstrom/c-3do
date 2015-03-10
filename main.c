#include "nano-bmp/include/nano_bmp.h"
#include "geometry.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sob.h"

typedef struct {
	uint8_t r; // 받기
	uint8_t g; // 세세세
	uint8_t b; // 메세지 받기
} rgb_color;

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color);
void render_mesh(struct model_t model, transform_3d transform, transform_3d view, float perspective, bmp_t *image, rgb_color color);
void draw_image(bmp_t *image);
void clear_image(bmp_t *image, rgb_color color);

int main(int argc, char** argv) {
	bmp_t *image = create_bmp(300, 300, 24);
	
	// NEED TO DEFINE COLORS
	rgb_color red = {255, 0, 0};
	rgb_color white = {255, 255, 255};
	rgb_color black = {0, 0, 0};

	clear_image(image, black);

	// load .sob-file
	FILE *fp = fopen("model/cube.sob", "r");
	// NEED TO check FILE POINTER
	if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
	}

	struct model_t model = load_model(fp);
	fclose(fp);

	// Make sure origin (0, 0) is drawn in the middle of the image
	transform_3d view = transform_3d_make_translation(image->info.w / 2.0, image->info.h / 2.0, 0.0);
	float perspective = 0.0025;

	// We want to scale the cube since its coordinate space goes from -1 to 1
	transform_3d scale = transform_3d_make_scale(50.0, 50.0, 50.0);

	// Move the cube up to the left
	transform_3d translate = transform_3d_make_translation(-70, -70, 0); 
	render_mesh(model, transform_3d_concat(scale, translate), view, perspective, image, red);
	
	// NEED TO UNLOAD MODULE
	unload_model(model);
	write_bmp("output.bmp", image);
	return 0;
}

/**
 Applies perspective to simulate vector positions in 3D-space, relative to a view position
 */
static inline vec3 apply_perspective(vec3 position, vec3 view_point, float amount, bmp_t *image) {
	float distance_x = view_point.x - position.x;
	float distance_y = view_point.y - position.y;
	position.x = position.x + position.z * distance_x * amount;
	position.y = position.y + position.z * distance_y * amount;
	return position;
}

void render_mesh(struct model_t model, transform_3d transform, transform_3d view, float perspective, bmp_t *image, rgb_color color) {
	for (int i = 0; i < model.num_edges; i++) {
		edge e = model.edges[i];

		// Position vertices in view and apply any transformations
		transform_3d t = transform_3d_concat(transform, view);
		vec3 v1 = transform_3d_apply(*e.v1, t);
		vec3 v2 = transform_3d_apply(*e.v2, t);

		// Calculate the view point, will be used to apply perspective
		vec3 view_point = {0, 0, 0};
		view_point = transform_3d_apply(view_point, view);

		// Apply perspective so that Z-position is reflected in a 2D image
		v1 = apply_perspective(v1, view_point, perspective, image);
		v2 = apply_perspective(v2, view_point, perspective, image);

		draw_line(v1, v2, image, color);
	}
}

void swapf(float *a, float *b) {
	float tmp = *a;
	*a = *b;
	*b = tmp;
}

void swapi(int *a, int *b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;	
}

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color) {

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

		// Make sure pixel is within bounds
		if (img_x > 0 && img_x <= image->info.w && img_y > 0 && img_y <= image->info.h) {
			set_pixel(image, img_x, img_y, color.r, color.g, color.b);
		}
	}
}

void clear_image(bmp_t *image, rgb_color color) {
	for (int x = 0; x < image->info.w; x++) {
		for (int y = 0; y < image->info.h; y++) {
			set_pixel(image, x, y, color.r, color.g, color.b);
		}
	}
}
