#ifndef _geometry_included
#define _geometry_included

typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3;

typedef struct edge {
	vec3 *v1;
	vec3 *v2;
} edge;

typedef struct transform_3d {
	float sx, ax, bx, tx;
	float ay, sy, by, ty;
	float az, bz, sz, tz;
	float am, bm, cm, dm;
} transform_3d;

transform_3d transform_3d_identity();
transform_3d transform_3d_make_translation(float tx, float ty, float sz);
transform_3d transform_3d_make_scale(float sx, float sy, float sz);
transform_3d transform_3d_concat(transform_3d t1, transform_3d t2);
vec3 transform_3d_apply(vec3 v, transform_3d t);

#endif