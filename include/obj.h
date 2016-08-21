#ifndef OBJ_H
#define OBJ_H

#include <stdio.h>
#include "geometry.h"

struct face {
	vec3 *vertices[3];
	vec3 *normals[3];
	vec2 *textures[3];
};

struct model {
	int num_vertices;
	int num_normals;
	int num_textures;
	int num_faces;

	vec3 *vertices;
	vec3 *normals;
	vec2 *textures;
	struct face *faces;
};

struct model load_model(FILE *fp);
void unload_model(struct model model);

#endif
