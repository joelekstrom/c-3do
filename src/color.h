#ifndef COLOR_H
#define COLOR_H

#include <inttypes.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

rgb_color interpolate_color(rgb_color c1, rgb_color c2, float value);
rgb_color scale_color(rgb_color color, float value);
rgb_color multiply_colors(rgb_color c1, rgb_color c2);
rgb_color add_color(rgb_color color, rgb_color other_color);
uint32_t rgba_from_color(rgb_color color);

#endif
