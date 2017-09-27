#ifndef SHADERS_H
#define SHADERS_H

#include "scene.h"

struct vertex {
	vec3 coordinate;
	rgb_color color;
	vec3 normal;
	vec2 texture_coordinate;
};

struct vertex_shader_input {
	struct vertex vertex;
	vec3 face_normal;
	transform_3d model;
	struct scene scene;
};

struct fragment_shader_input {
	struct vertex interpolated_v;
	struct texture *texture;
	struct texture *normal_map;
};

// Vertex shaders
struct vertex goraud_shader(struct vertex_shader_input input);
struct vertex flat_shader(struct vertex_shader_input input);

// Fragment shaders
rgb_color apply_texture_shader(struct fragment_shader_input input);

#endif
