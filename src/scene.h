#ifndef SCENE_H
#define SCENE_H

#include "color.h"
#include "geometry.h"

struct directional_light {
	vec3 direction;
	rgb_color intensity;
};

struct scene {
	transform_3d view;
	double perspective;
	rgb_color ambient_light;
	struct directional_light *directional_lights;
	int directional_light_count;
};

#endif
