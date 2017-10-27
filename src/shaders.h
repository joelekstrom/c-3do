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
	struct scene scene;
};

typedef struct vertex vertex_shader;
typedef rgb_color fragment_shader;

// Vertex shaders
vertex_shader goraud_shader(struct vertex_shader_input input);
vertex_shader flat_shader(struct vertex_shader_input input);

// Fragment shaders
fragment_shader apply_texture_shader(struct fragment_shader_input input);

#endif
