#include "nano-bmp/include/nano_bmp.h"
#include "geometry.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sob.h"

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color);
void render_mesh(struct model_t model, vec3 translation, vec3 view, float perspective, bmp_t *image, rgb_color color);
void draw_image(bmp_t *image);
void clear_image(bmp_t *image, rgb_color color);

int main(int argc, char** argv) {
	bmp_t *image = create_bmp(300, 300, 24);
	
	rgb_color red = {255, 0, 0};
	rgb_color white = {255, 255, 255};
	rgb_color black = {0, 0, 0};

	clear_image(image, black);

	// load .jobj-file
	FILE *fp = fopen("model/cube.sob", "r");
	if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
	}

	struct model_t model = load_model(fp);
	fclose(fp);

	// Position camera in the middle of the image
	vec3 view = { image->info.w / 2, image->info.h / 2, 0 };
	float perspective = 0.0025;
	vec3 translation = {0, 0, 0};

	render_mesh(model, translation, view, perspective, image, red);

	translation.x = 105;
	render_mesh(model, translation, view, perspective, image, red);

	translation.x = 170;
	translation.y = 120;
	translation.z = 20;
	render_mesh(model, translation, view, perspective, image, red);

	translation.x = 10;
	translation.y = 150;
	translation.z = 80;
	render_mesh(model, translation, view, perspective, image, red);

	unload_model(model);
	write_bmp("output.bmp", image);
	return 0;
}

/**
 Translates a single vector
 */
static inline vec3 translate(vec3 v, vec3 translation) {
	v.x = v.x + translation.x;
	v.y = v.y + translation.y;
	v.z = v.z + translation.z;
	return v;
}

/**
 Applies perspective to simulate vector positions in 3D-space, relative to a view position
 */
static inline vec3 apply_perspective(vec3 position, vec3 view, float amount, bmp_t *image) {
	float distance_x = view.x - position.x;
	float distance_y = view.y - position.y;
	position.x = position.x + position.z * distance_x * amount;
	position.y = position.y + position.z * distance_y * amount;
	return position;
}

void render_mesh(struct model_t model, vec3 translation, vec3 view, float perspective, bmp_t *image, rgb_color color) {
	for (int i = 0; i < model.num_edges; i++) {
		edge e = model.edges[i];

		// Apply any translation
		vec3 v1 = translate(*e.v1, translation);
		vec3 v2 = translate(*e.v2, translation);

		// Apply perspective so that Z-position is reflected in a 2D image
		v1 = apply_perspective(v1, view, perspective, image);
		v2 = apply_perspective(v2, view, perspective, image);

		draw_line(v1, v2, image, color);
	}
}

void swap_int(int *a, int *b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color) {

	// If the line is steep (height > width), we transpose the line, so we can always loop on x-value
	int steep = abs(p2.y - p1.y) > abs(p2.x - p1.x);
    if (steep) {
    	swap_int(&p1.x, &p1.y);
    	swap_int(&p2.x, &p2.y);
    }

    // Make sure it's drawn left->right
    if (p2.x <= p1.x) {
    	swap_int(&p1.x, &p2.x);
    	swap_int(&p1.y, &p2.y);
    }

    for (int x = p1.x; x <= p2.x; x++) {
		float t = (x - p1.x) / (float)(p2.x - p1.x);
		int y = p1.y * (1.0 - t) + (p2.y * t) + 0.5;

		// De-transpose if needed
		if (steep) {
			set_pixel(image, y, x, color.r, color.g, color.b);
		} else {
			set_pixel(image, x, y, color.r, color.g, color.b);
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
