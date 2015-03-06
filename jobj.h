#ifndef _jobj_included
#define _jobj_included

#include <stdio.h>
#include "geometry.h"

struct model_t {
	int num_vertices;
	int num_edges;

	vec3 *vertices;
	edge *edges;
};

struct model_t load_model(FILE *fp);
void unload_model(struct model_t model);

#endif