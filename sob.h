/**
 A parser for .sob - the Simple Object Format
 */
#ifndef _sob_included
#define _sob_included

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