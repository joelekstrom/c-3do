#ifndef OBJ_H
#define OBJ_H

#include <stdio.h>
#include "geometry.h"

struct model_t {
	int num_vertices;
	int num_edges;
	int num_faces;

	vec3 *vertices;
	edge *edges;
	face *faces;
};

struct model_t load_model(FILE *fp);
void unload_model(struct model_t model);

#endif