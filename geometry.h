#ifndef _geometry_included
#define _geometry_included

typedef struct vec3 {
	int x;
	int y;
	int z;
} vec3;

typedef struct edge {
	vec3 v1;
	vec3 v2;
} edge;

#endif