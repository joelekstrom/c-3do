#ifndef OBJ_H
#define OBJ_H

#include <stdio.h>
#include "geometry.h"

struct face {
	vec3 *vertices[3];
	vec3 *normals[3];
	vec3 *textures[3];
};

struct model_t {
	int num_vertices;
	int num_normals;
	int num_textures;
	int num_faces;

	vec3 *vertices;
	vec3 *normals;
	vec3 *textures;
	struct face *faces;
};

struct model_t load_model(FILE *fp);
void unload_model(struct model_t model);

#endif