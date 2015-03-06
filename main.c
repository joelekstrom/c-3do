#include "nano-bmp/include/nano_bmp.h"
#include "geometry.h"
#include <stdlib.h>
#include <stdio.h>
#include "sob.h"

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color);
void draw_edges(edge edges[], int edge_count, bmp_t *image, rgb_color color);
void draw_image(bmp_t *image);
void clear_image(bmp_t *image, rgb_color color);

int main(int argc, char** argv) {
	bmp_t *image = create_bmp(150, 100, 24);
	
	rgb_color red = {255, 0, 0};
	rgb_color white = {255, 255, 255};
	rgb_color black = {0, 0, 0};

	clear_image(image, black);

	// load .jobj-file
	FILE *fp = fopen("model/fsharp.sob", "r");
	if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
	}

	struct model_t model = load_model(fp);
	fclose(fp);

	draw_edges(model.edges, model.num_edges, image, red);
	unload_model(model);
	write_bmp("output.bmp", image);
	return 0;
}

void draw_edges(edge edges[], int edge_count, bmp_t *image, rgb_color color) {
	for (int i = 0; i < edge_count; i++) {
		edge e = edges[i];
		draw_line(*e.v1, *e.v2, image, color);
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
		int y = p1.y * (1.0 - t) + (p1.y * t) + 0.5;

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
