#include "color.h"

rgb_color interpolate_color(rgb_color c1, rgb_color c2, float value) {
	rgb_color result = { .r = c1.r + (c2.r - c1.r) * value,
                         .g = c1.g + (c2.g - c1.g) * value,
                         .b = c1.b + (c2.b - c1.b) * value };
	return result;
}

rgb_color multiply_color(rgb_color color, float value) {
	rgb_color result = { .r = color.r * value,
                         .g = color.g * value,
                         .b = color.b * value };
	return result;
}

uint32_t rgba_from_color(rgb_color color) {
	uint32_t result = 0;
	result |= (color.r << 24);
	result |= (color.g << 16);
	result |= (color.b << 8);
	result |= 0xff;
	return result;
}
