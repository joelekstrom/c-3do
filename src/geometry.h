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

float dot_product_2d(vec2 v1, vec2 v2);
float dot_product_3d(vec3 v1, vec3 v2);
vec3 cross_product(vec3 v1, vec3 v2);

vec3 vec3_unit(vec3 v);
vec3 vec3_subtract(vec3 v1, vec3 v2);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_scale(vec3 v, float s);

// Linear interpolation
float flerp(float a, float b, float value);
vec2 vec2_lerp(vec2 a, vec2 b, float value);
vec3 vec3_lerp(vec3 a, vec3 b, float value);

// Transforms
extern const transform_3d transform_3d_identity;
transform_3d transform_3d_make_translation(float tx, float ty, float sz);
transform_3d transform_3d_make_scale(float sx, float sy, float sz);
transform_3d transform_3d_make_rotation(float rx, float ry, float rz);
transform_3d transform_3d_concat(transform_3d t1, transform_3d t2);
vec3 transform_3d_apply(vec3 v, transform_3d t);

#endif