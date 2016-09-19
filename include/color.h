#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

rgb_color interpolate_color(rgb_color c1, rgb_color c2, float value);
rgb_color multiply_color(rgb_color color, float value);

#endif
