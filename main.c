#include <stdio.h>
#include "Nano-BMP/include/nano_bmp.h"
#include "jobj.h"
#include "geometry.h"

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
	FILE *fp = fopen("model.jobj", "r");
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

void draw_line(struct vec3 p1, struct vec3 p2, bmp_t *image, rgb_color color) {

  	// steep is true if height > width
	int steep = p2.y - p1.y > p2.x - p1.x;

	if (steep) {
		for (int y = p1.y; y < p2.y; y++) {
			float t = (float)(y - p1.y) / (float)(p2.y - p1.y);
			int x = (p2.x - p1.x) * t + p1.x + 0.5;
			set_pixel(image, x, y, color.r, color.g, color.b);
		}
	} else {
		for (int x = p1.x; x < p2.x; x++) {
			float t = (float)(x - p1.x) / (float)(p2.x - p1.x);
			int y = (p2.y - p1.y) * t + p1.y + 0.5;
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
