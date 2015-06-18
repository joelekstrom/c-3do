#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct vec2 {
	float x;
	float y;
} vec2;

typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3;

typedef struct transform_3d {
	float sx, ax, bx, tx;
	float ay, sy, by, ty;
	float az, bz, sz, tz;
	float am, bm, cm, dm;
} transform_3d;

// Math
float dot_product_2d(vec2 v1, vec2 v2);
float dot_product_3d(vec3 v1, vec3 v2);
vec3 cross_product(vec3 v1, vec3 v2);

vec3 vec3_unit(vec3 v);
vec3 vec3_subtract(vec3 v1, vec3 v2);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_neg(vec3 v);

// Transforms
transform_3d transform_3d_identity();
transform_3d transform_3d_make_translation(float tx, float ty, float sz);
transform_3d transform_3d_make_scale(float sx, float sy, float sz);
transform_3d transform_3d_concat(transform_3d t1, transform_3d t2);
vec3 transform_3d_apply(vec3 v, transform_3d t);

// Vertex sorting
void sort_vertices_y(vec3 vertices[], int count);
void get_bounding_box_2d(vec2 vertices[], int count, float *min_x, float *min_y, float *max_x, float *max_y);

#endif