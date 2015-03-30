#include "geometry.h"
#include <stdlib.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

typedef enum {
	BMP_CONTEXT_TYPE
} context_type;

struct graphics_context {
	context_type type;
	int width;
	int height;
	void *_internal;
};

struct graphics_context *create_context(context_type type, int width, int height);
void destroy_context(struct graphics_context *context);

void draw_line(vec2 p1, vec2 p2, struct graphics_context *context, rgb_color color);
void fill_triangle(vec2 p1, vec2 p2, vec2 p3, struct graphics_context *context, rgb_color color);
void goraud_triangle(vec2 p1, vec2 p2, vec2 p3, rgb_color c1, rgb_color c2, rgb_color c3, struct graphics_context *context);
void clear(struct graphics_context *context, rgb_color color);
void bmp_context_save(struct graphics_context *context, char name[]);