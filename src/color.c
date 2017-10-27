#include "color.h"

rgb_color interpolate_color(rgb_color c1, rgb_color c2, double value) {
	rgb_color result = { .r = c1.r + (c2.r - c1.r) * value,
                         .g = c1.g + (c2.g - c1.g) * value,
                         .b = c1.b + (c2.b - c1.b) * value };
	return result;
}

rgb_color scale_color(rgb_color color, double value) {
	rgb_color result = { .r = color.r * value,
                         .g = color.g * value,
                         .b = color.b * value };
	return result;
}

rgb_color multiply_colors(rgb_color c1, rgb_color c2) {
	rgb_color result = { .r = c1.r * (c2.r / 255.0),
                         .g = c1.g * (c2.g / 255.0),
                         .b = c1.b * (c2.b / 255.0)};
	return result;
}

rgb_color add_color(rgb_color color, rgb_color other_color) {
	int r = color.r + other_color.r;
	int g = color.g + other_color.g;
	int b = color.b + other_color.b;
	rgb_color result = {r <= 255 ? r : 255, g <= 255 ? g : 255, b <= 255 ? b : 255};
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
